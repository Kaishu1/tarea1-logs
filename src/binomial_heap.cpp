#include "binomial_heap.h"

#include <cmath>
#include <stdexcept>
#include <utility>

// Constructor de la cola de prioridad binomial
BinomialHeap::BinomialHeap(std::size_t cantidad_vertices)
    : raices(nullptr), handles(cantidad_vertices, nullptr), cantidad(0) {}

BinomialHeap::BinomialHeap(const std::vector<double>& costos)
    : BinomialHeap(costos.size()) {
    for (std::size_t v = 0; v < costos.size(); ++v) {
        insert(costos[v], static_cast<int>(v));
    }
}

// Destructor de la cola de prioridad binomial
BinomialHeap::~BinomialHeap() {
    destroyTrees(raices);
}

// linkTrees(Q, x, y)
// qué hace: unir dos árboles del mismo grado y devolver su nueva raíz
BinomialHeap::Node* BinomialHeap::linkTrees(Node* primero, Node* segundo) {
    if (segundo->content.key < primero->content.key) {
        std::swap(primero, segundo);
    }
    segundo->parent = primero;
    segundo->hermano = primero->hijo;
    primero->hijo = segundo;
    ++primero->grado;
    return primero;
}

// insert(Q, v, c)
// qué hace: insertar el par (costo, vértice) en la cola
void BinomialHeap::insert(double costo, int vertice) {
    if (vertice < 0 || static_cast<std::size_t>(vertice) >= handles.size()) {
        throw std::out_of_range("Vertice fuera de rango");
    }
    if (handles[vertice] != nullptr) {
        throw std::invalid_argument("El vertice ya esta en la cola");
    }
    if (std::isnan(costo)) {
        throw std::invalid_argument("El costo no puede ser NaN");
    }

    Node* nuevo = new Node{{costo, vertice}, 0, nullptr, nullptr, nullptr};
    handles[vertice] = nuevo;

    // Cada enlace propaga un acarreo; no se recorren las raíces restantes.
    while (raices != nullptr && raices->grado == nuevo->grado) {
        Node* siguiente = raices->hermano;
        raices->hermano = nullptr;
        nuevo = linkTrees(nuevo, raices);
        raices = siguiente;
    }
    nuevo->hermano = raices;
    raices = nuevo;
    ++cantidad;
}

// unionRoots(Q, r1, r2)
// qué hace: mezclar raíces ordenadas por grado y enlazar árboles de igual grado
BinomialHeap::Node* BinomialHeap::unionRoots(Node* primera, Node* segunda) {
    Node auxiliar{};
    Node* ultimo = &auxiliar;
    while (primera != nullptr && segunda != nullptr) {
        if (primera->grado <= segunda->grado) {
            ultimo->hermano = primera;
            primera = primera->hermano;
        } else {
            ultimo->hermano = segunda;
            segunda = segunda->hermano;
        }
        ultimo = ultimo->hermano;
    }
    ultimo->hermano = (primera != nullptr) ? primera : segunda;

    Node* cabeza = auxiliar.hermano;
    Node* anterior = nullptr;
    Node* actual = cabeza;
    while (actual != nullptr && actual->hermano != nullptr) {
        Node* siguiente = actual->hermano;
        // Con tres grados iguales, avanzamos para enlazar los últimos dos.
        if (actual->grado != siguiente->grado ||
            (siguiente->hermano != nullptr &&
             siguiente->hermano->grado == actual->grado)) {
            anterior = actual;
            actual = siguiente;
        } else {
            Node* resto = siguiente->hermano;
            actual = linkTrees(actual, siguiente);
            actual->hermano = resto;
            if (anterior == nullptr) {
                cabeza = actual;
            } else {
                anterior->hermano = actual;
            }
        }
    }
    return cabeza;
}

// extractMin(Q)
// qué hace: obtener y eliminar el par de menor costo
std::pair<double, int> BinomialHeap::extractMin() {
    if (empty()) {
        throw std::underflow_error("La cola esta vacia");
    }

    Node* minimo = raices;
    Node* anteriorMinimo = nullptr;
    Node* anterior = nullptr;
    for (Node* actual = raices; actual != nullptr; actual = actual->hermano) {
        if (actual->content.key < minimo->content.key) {
            minimo = actual;
            anteriorMinimo = anterior;
        }
        anterior = actual;
    }

    if (anteriorMinimo == nullptr) {
        raices = minimo->hermano;
    } else {
        anteriorMinimo->hermano = minimo->hermano;
    }

    // Los hijos pasan a ser raíces, en orden creciente de grado.
    Node* hijos = nullptr;
    Node* actual = minimo->hijo;
    while (actual != nullptr) {
        Node* siguiente = actual->hermano;
        actual->parent = nullptr;
        actual->hermano = hijos;
        hijos = actual;
        actual = siguiente;
    }
    raices = unionRoots(raices, hijos);

    std::pair<double, int> resultado{minimo->content.key, minimo->content.vertice};
    handles[minimo->content.vertice] = nullptr;
    --cantidad;
    delete minimo;
    return resultado;
}

// decreaseKey(Q, v, c)
// qué hace: acceder al par que representa al nodo v y reducir su costo a c
std::size_t BinomialHeap::decreaseKey(int v, double c) {
    if (v < 0 || static_cast<std::size_t>(v) >= handles.size()) {
        throw std::out_of_range("Vertice fuera de rango");
    }
    Node* x = handles[v];
    if (x == nullptr) {
        throw std::invalid_argument("El vertice no esta en la cola");
    }
    if (std::isnan(c) || c > x->content.key) {
        throw std::invalid_argument("El nuevo costo debe ser menor o igual al actual");
    }

    std::size_t intercambios = 0;
    x->content.key = c;
    Node* y = x->parent;
    while (y != nullptr && x->content.key < y->content.key) {
        std::swap(x->content, y->content);
        // Ambos nodos representan otros vértices después del intercambio.
        handles[x->content.vertice] = x;
        handles[y->content.vertice] = y;
        ++intercambios;
        x = y;
        y = x->parent;
    }
    return intercambios;
}

// empty(Q)
// qué hace: determinar si la cola está vacía
bool BinomialHeap::empty() const {
    return cantidad == 0;
}

// size(Q)
// qué hace: obtener la cantidad de pares (costo, vértice) en la cola
std::size_t BinomialHeap::size() const {
    return cantidad;
}

// roots(Q)
// qué hace: obtener el puntero al primer nodo raíz de la cola
const BinomialHeap::Node* BinomialHeap::roots() const {
    return raices;
}

// handle(Q, v)
// qué hace: obtener el puntero al nodo que representa al vértice v
const BinomialHeap::Node* BinomialHeap::handle(int vertice) const {
    if (vertice < 0 || static_cast<std::size_t>(vertice) >= handles.size()) {
        throw std::out_of_range("Vertice fuera de rango");
    }
    return handles[vertice];
}

// destroyTrees(Q, r)
// qué hace: liberar memoria de todos los nodos en el árbol con raíz r
void BinomialHeap::destroyTrees(Node* raiz) {
    while (raiz != nullptr) {
        Node* siguiente = raiz->hermano;
        destroyTrees(raiz->hijo);
        delete raiz;
        raiz = siguiente;
    }
}
