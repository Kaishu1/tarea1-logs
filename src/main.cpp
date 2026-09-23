#include "binomial_heap.h"

#include <cassert>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>

// Revisa forma binomial, orden de heap y handles; devuelve el tamaño del árbol.
std::size_t verificarArbol(const BinomialHeap& cola,
                          const BinomialHeap::Node* nodo,
                          std::vector<bool>& visitados) {
    assert(nodo->content.vertice >= 0);
    assert(static_cast<std::size_t>(nodo->content.vertice) < visitados.size());
    assert(!visitados[nodo->content.vertice]);
    visitados[nodo->content.vertice] = true;
    assert(cola.handle(nodo->content.vertice) == nodo);

    std::size_t cantidad = 1;
    int gradoEsperado = nodo->grado - 1;
    for (const auto* hijo = nodo->hijo; hijo != nullptr; hijo = hijo->hermano) {
        assert(hijo->parent == nodo);
        assert(hijo->content.key >= nodo->content.key);
        assert(hijo->grado == gradoEsperado);
        --gradoEsperado;
        cantidad += verificarArbol(cola, hijo, visitados);
    }
    assert(gradoEsperado == -1);
    assert(cantidad == (std::size_t{1} << nodo->grado));
    return cantidad;
}

// Revisa raíces con grados crecientes y presencia de cada vértice esperado.
void verificarCola(const BinomialHeap& cola, const std::vector<double>& costos) {
    std::vector<bool> visitados(costos.size(), false);
    std::size_t cantidad = 0;
    int gradoAnterior = -1;
    for (const auto* raiz = cola.roots(); raiz != nullptr; raiz = raiz->hermano) {
        assert(raiz->parent == nullptr);
        assert(raiz->grado > gradoAnterior);
        gradoAnterior = raiz->grado;
        cantidad += verificarArbol(cola, raiz, visitados);
    }
    assert(cantidad == cola.size());
    assert(cantidad == costos.size());
    assert(cola.empty() == costos.empty());
    for (std::size_t v = 0; v < costos.size(); ++v) {
        assert(visitados[v]);
        assert(cola.handle(static_cast<int>(v))->content.key == costos[v]);
    }
}

int main() {
    const double infinito = std::numeric_limits<double>::infinity();
    const std::vector<std::vector<double>> casos = { // datitos de prueba
        {}, {0.5}, {0.8, 0.2},
        {0.9, 0.8, 0.7, 0.6, 0.5, 0.4, 0.3, 0.2},
        {0.4, 0.4, 0.1, 0.7, 0.1},
        {infinito, infinito, 0.0, infinito, infinito}
    };
    for (const auto& costos : casos) {
        BinomialHeap cola(costos);
        verificarCola(cola, costos);
    }

    BinomialHeap incremental(17);
    std::vector<double> costos;
    for (int v = 0; v < 17; ++v) {
        assert(incremental.handle(v) == nullptr);
        costos.push_back((17 - v) / 17.0);
        incremental.insert(costos.back(), v);
        verificarCola(incremental, costos);
    }

    bool duplicadoRechazado = false;
    try {
        incremental.insert(0.1, 0);
    } catch (const std::invalid_argument&) {
        duplicadoRechazado = true;
    }
    assert(duplicadoRechazado);

    bool indiceRechazado = false;
    try {
        incremental.insert(0.1, 17);
    } catch (const std::out_of_range&) {
        indiceRechazado = true;
    }
    assert(indiceRechazado);
    verificarCola(incremental, costos);

    std::cout << "Pruebas de construccion e insercion: OK\n";
    return 0;
}
