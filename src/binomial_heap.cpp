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
