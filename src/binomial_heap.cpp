#include "binomial_heap.h"

#include <cmath>
#include <stdexcept>
#include <utility>

// Crea una cola vacía con handles para cantidad_vertices índices.
BinomialHeap::BinomialHeap(std::size_t cantidad_vertices)
    : raices(nullptr), handles(cantidad_vertices, nullptr), cantidad(0) {}

// Inicializa la cola con un elemento por cada costo del vector.
BinomialHeap::BinomialHeap(const std::vector<double>& costos)
    : BinomialHeap(costos.size()) {
    for (std::size_t v = 0; v < costos.size(); ++v) {
        insert(costos[v], static_cast<int>(v));
    }
}

// Libera todos los árboles de la cola.
BinomialHeap::~BinomialHeap() {
    destroyTrees(raices);
}

// Enlaza dos árboles del mismo grado y devuelve como raíz el de menor clave.
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

// Inserta (costo, vertice); rechaza índices inválidos, duplicados y claves NaN.
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

// Mezcla listas de raíces ordenadas por grado y devuelve la lista consolidada.
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

// Elimina y devuelve (clave, vértice) mínimo; lanza underflow_error si está vacía.
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

// Reduce la clave de v a c y devuelve los intercambios usados para restaurar el orden.
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

// Indica si la cola no contiene elementos.
bool BinomialHeap::empty() const {
    return cantidad == 0;
}

// Devuelve la cantidad de elementos.
std::size_t BinomialHeap::size() const {
    return cantidad;
}

// Devuelve la primera raíz o nullptr si no hay árboles.
const BinomialHeap::Node* BinomialHeap::roots() const {
    return raices;
}

// Devuelve el nodo de vertice o nullptr si no está en la cola; valida el índice.
const BinomialHeap::Node* BinomialHeap::handle(int vertice) const {
    if (vertice < 0 || static_cast<std::size_t>(vertice) >= handles.size()) {
        throw std::out_of_range("Vertice fuera de rango");
    }
    return handles[vertice];
}

// Libera recursivamente los árboles enlazados desde raiz.
void BinomialHeap::destroyTrees(Node* raiz) {
    while (raiz != nullptr) {
        Node* siguiente = raiz->hermano;
        destroyTrees(raiz->hijo);
        delete raiz;
        raiz = siguiente;
    }
}
