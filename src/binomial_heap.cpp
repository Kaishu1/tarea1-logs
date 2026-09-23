#include "binomial_heap.h"

#include <cmath>
#include <stdexcept>
#include <utility>

BinomialHeap::BinomialHeap(std::size_t cantidad_vertices)
    : raices(nullptr), handles(cantidad_vertices, nullptr), cantidad(0) {}

BinomialHeap::BinomialHeap(const std::vector<double>& costos)
    : BinomialHeap(costos.size()) {
    for (std::size_t v = 0; v < costos.size(); ++v) {
        insert(costos[v], static_cast<int>(v));
    }
}

BinomialHeap::~BinomialHeap() {
    destroyTrees(raices);
}

BinomialHeap::Node* BinomialHeap::linkTrees(Node* primero, Node* segundo) {
    if (segundo->costo < primero->costo) {
        std::swap(primero, segundo);
    }
    segundo->padre = primero;
    segundo->hermano = primero->hijo;
    primero->hijo = segundo;
    ++primero->grado;
    return primero;
}

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

    Node* nuevo = new Node{costo, vertice, 0, nullptr, nullptr, nullptr};
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
        if (actual->costo < minimo->costo) {
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
        actual->padre = nullptr;
        actual->hermano = hijos;
        hijos = actual;
        actual = siguiente;
    }
    raices = unionRoots(raices, hijos);

    std::pair<double, int> resultado{minimo->costo, minimo->vertice};
    handles[minimo->vertice] = nullptr;
    --cantidad;
    delete minimo;
    return resultado;
}

// decreaseKey(Q, v, k)
// qué hace: acceder al par que representa al nodo v y reducir su costo a c
std::size_t BinomialHeap::decreaseKey(int vertice, double nuevoCosto) {
    if (vertice < 0 || static_cast<std::size_t>(vertice) >= handles.size()) {
        throw std::out_of_range("Vertice fuera de rango");
    }
    Node* actual = handles[vertice];
    if (actual == nullptr) {
        throw std::invalid_argument("El vertice no esta en la cola");
    }
    if (std::isnan(nuevoCosto) || nuevoCosto > actual->costo) {
        throw std::invalid_argument("El nuevo costo debe ser menor o igual al actual");
    }

    actual->costo = nuevoCosto;
    std::size_t intercambios = 0;
    Node* padre = actual->padre;
    while (padre != nullptr && actual->costo < padre->costo) {
        std::swap(actual->costo, padre->costo);
        std::swap(actual->vertice, padre->vertice);
        // Ambos nodos representan otros vértices después del intercambio.
        handles[actual->vertice] = actual;
        handles[padre->vertice] = padre;
        ++intercambios;
        actual = padre;
        padre = actual->padre;
    }
    return intercambios;
}

bool BinomialHeap::empty() const {
    return cantidad == 0;
}

std::size_t BinomialHeap::size() const {
    return cantidad;
}

const BinomialHeap::Node* BinomialHeap::roots() const {
    return raices;
}

const BinomialHeap::Node* BinomialHeap::handle(int vertice) const {
    if (vertice < 0 || static_cast<std::size_t>(vertice) >= handles.size()) {
        throw std::out_of_range("Vertice fuera de rango");
    }
    return handles[vertice];
}

void BinomialHeap::destroyTrees(Node* raiz) {
    while (raiz != nullptr) {
        Node* siguiente = raiz->hermano;
        destroyTrees(raiz->hijo);
        delete raiz;
        raiz = siguiente;
    }
}
