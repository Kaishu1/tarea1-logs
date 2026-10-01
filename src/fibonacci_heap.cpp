#include "fibonacci_heap.h"

#include <cmath>
#include <stdexcept>
#include <utility>

FibonacciHeap::FibonacciHeap(std::size_t cantidad_vertices)
    : minimo(nullptr),
      handles(cantidad_vertices, nullptr),
      cantidad(0) {}

FibonacciHeap::FibonacciHeap(const std::vector<double>& costos)
    : FibonacciHeap(costos.size()) {
    for (std::size_t v = 0; v < costos.size(); ++v) {
        insert(costos[v], static_cast<int>(v));
    }
}

FibonacciHeap::~FibonacciHeap() {
    if (minimo != nullptr) {
        destroy(minimo);
    }
}

void FibonacciHeap::addToRootList(Node* nodo) {
    if (minimo == nullptr) {
        nodo->left = nodo;
        nodo->right = nodo;
        minimo = nodo;
    } else {
        nodo->left = minimo;
        nodo->right = minimo->right;
        minimo->right->left = nodo;
        minimo->right = nodo;

        if (nodo->content.key < minimo->content.key) {
            minimo = nodo;
        }
    }
}

void FibonacciHeap::insert(double costo, int vertice) {
    if (vertice < 0 || static_cast<std::size_t>(vertice) >= handles.size()) {
        throw std::out_of_range("Vertice fuera de rango");
    }

    if (handles[vertice] != nullptr) {
        throw std::invalid_argument("El vertice ya esta en la cola");
    }

    if (std::isnan(costo)) {
        throw std::invalid_argument("El costo no puede ser NaN");
    }

    Node* nuevo_nodo = new Node{Content{costo, vertice}, nullptr, nullptr, nullptr, nullptr, 0, false};
    addToRootList(nuevo_nodo);
    handles[vertice] = nuevo_nodo;
    ++cantidad;
}

void FibonacciHeap::removeFromList(Node* nodo) {
    nodo->left->right = nodo->right;
    nodo->right->left = nodo->left;

    nodo->left = nodo;
    nodo->right = nodo;
}

std::pair<double, int> FibonacciHeap::extractMin() {
    if (empty()) {
        throw std::underflow_error("La cola esta vacia");
    }

    Node* nodo_minimo = minimo;
    
    std::pair<double, int> resultado{
        nodo_minimo->content.key,
        nodo_minimo->content.vertice
    };

    // Mover los hijos del nodo mínimo a la lista de raíces
    if (nodo_minimo->child != nullptr) {
        Node* hijo = nodo_minimo->child;
        Node* actual = hijo;
        do {
            Node* siguiente_hijo = actual->right;
            
            actual->parent = nullptr;
            actual->marked = false;

            // Desconectar de la lista de hijos.
            actual->left = actual;
            actual->right = actual;

            // Agregar como raiz
            addToRootList(actual);
            actual = siguiente_hijo;
        } while (actual != hijo);

        nodo_minimo->child = nullptr;
        nodo_minimo->degree = 0;
    }

    // Eliminar el nodo mínimo de la lista de raíces
    if (nodo_minimo->right == nodo_minimo) {
        minimo = nullptr;
    } else {
        Node* siguiente = nodo_minimo->right;
        removeFromList(nodo_minimo);
        minimo = siguiente;
    }

    handles[nodo_minimo->content.vertice] = nullptr;
    --cantidad;

    delete nodo_minimo;

    // Consolidar los árboles en la lista de raíces
    if (minimo != nullptr) {
        consolidate();
    }

    return resultado;
}

void FibonacciHeap::consolidate() {
    if (minimo == nullptr) {
        return;
    }

    std::vector<Node*> tabla(64, nullptr); // Tamaño suficiente para la mayoría de los casos

    Node* actual = minimo;

    std::vector<Node*> raices;

    do {
        raices.push_back(actual);
        actual = actual->right;
    } while (actual != minimo);

    // Aislar todas las raíces antes de comenzar consolidación
    for (Node* nodo : raices) {
        nodo->left = nodo;
        nodo->right = nodo;
    }

    minimo = nullptr;

    // Combinar árboles de el mismo grado
    for (Node* x : raices) {
        int degree = x->degree;

        while (tabla[degree] != nullptr) {
            Node* y = tabla[degree];

            if (x->content.key > y->content.key) {
                std::swap(x, y);
            }

            link(y, x);
            tabla[degree] = nullptr;
            ++degree;
        }

        tabla[degree] = x;
    }

    // Reconstruir la lista de raíces y encontrar el nuevo mínimo
    for (Node* nodo : tabla) {
        if (nodo != nullptr) {
            nodo->left = nodo;
            nodo->right = nodo;    
            addToRootList(nodo);
        }
    }
}

void FibonacciHeap::link(Node* child, Node* parent) {
    removeFromList(child);

    child->parent = parent;
    child->marked = false;

    if (parent->child == nullptr) {
        parent->child = child;
        child->left = child;
        child->right = child;
    } else {
        Node* primero = parent->child;

        child->left = primero;
        child->right = primero->right;

        primero->right->left = child;
        primero->right = child;
    }

    ++parent->degree;
}

std::size_t FibonacciHeap::decreaseKey(int v, double c) {
    if (v < 0 || static_cast<std::size_t>(v) >=handles.size()) {
        throw std::out_of_range("Vertice fuera de rango");
    }

    Node* x = handles[v];

    if (x == nullptr) {
        throw std::invalid_argument("El vertice no esta en la cola");
    }

    if (std::isnan(c)) {
        throw std::invalid_argument("El costo no puede ser NaN");
    }

    if (c > x->content.key) {
        throw std::invalid_argument("El nuevo costo debe ser menor o igual al costo actual");
    }

    x->content.key = c;

    Node* y = x->parent;
    std::size_t num_cuts = 0;

    if (y != nullptr && x->content.key < y->content.key) {
        cut(x, y);
        ++num_cuts;
        
        // Contar los cortes en cascada si es necesario
        num_cuts += cascadingCut(y);
    }

    if (x->content.key < minimo->content.key) {
        minimo = x;
    }
    return num_cuts;
}

void FibonacciHeap::cut(Node* x, Node* y) {
    // Eliminar x de la lista de hijos de y
    if (x->right == x) {
        y->child = nullptr;
    } else {
        if (y->child == x) {
            y->child = x->right;
        }
        removeFromList(x);
    }

    --y->degree;

    // x pasa a ser raíz
    x->parent = nullptr;
    x->marked = false;

    addToRootList(x);
}

std::size_t FibonacciHeap::cascadingCut(Node* y) {
    Node* z = y->parent;

    if (z != nullptr) {
        return 0;
    }

    if (!y->marked) {
        y->marked = true;
        return 0;
    } else {
        cut(y, z);
        return 1 + cascadingCut(z);
    }
}

bool FibonacciHeap::empty() const {
    return cantidad == 0;
}

std::size_t FibonacciHeap::size() const {
    return cantidad;
}

const FibonacciHeap::Node* FibonacciHeap::handle(int vertice) const {
    if (vertice < 0 || static_cast<std::size_t>(vertice) >= handles.size()) {
        throw std::out_of_range("Vertice fuera de rango");
    }
    return handles[vertice];
}

void FibonacciHeap::destroy(Node* nodo) {
    if (nodo == nullptr) {
        return;
    }

    Node* actual = nodo;
    do {
        Node* siguiente = actual->right;

        if (actual->child != nullptr) {
            destroy(actual->child);
        }

        delete actual;
        actual = siguiente;
    } while (actual != nodo);
}