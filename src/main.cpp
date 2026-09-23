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
    assert(nodo->vertice >= 0);
    assert(static_cast<std::size_t>(nodo->vertice) < visitados.size());
    assert(!visitados[nodo->vertice]);
    visitados[nodo->vertice] = true;
    assert(cola.handle(nodo->vertice) == nodo);

    std::size_t cantidad = 1;
    int gradoEsperado = nodo->grado - 1;
    for (const auto* hijo = nodo->hijo; hijo != nullptr; hijo = hijo->hermano) {
        assert(hijo->padre == nodo);
        assert(hijo->costo >= nodo->costo);
        assert(hijo->grado == gradoEsperado);
        --gradoEsperado;
        cantidad += verificarArbol(cola, hijo, visitados);
    }
    assert(gradoEsperado == -1);
    assert(cantidad == (std::size_t{1} << nodo->grado));
    return cantidad;
}

// Revisa raíces con grados crecientes y presencia de cada vértice esperado.
void verificarCola(const BinomialHeap& cola, const std::vector<double>& costos,
                   const std::vector<bool>& presentes) {
    std::vector<bool> visitados(costos.size(), false);
    std::size_t cantidad = 0;
    int gradoAnterior = -1;
    for (const auto* raiz = cola.roots(); raiz != nullptr; raiz = raiz->hermano) {
        assert(raiz->padre == nullptr);
        assert(raiz->grado > gradoAnterior);
        gradoAnterior = raiz->grado;
        cantidad += verificarArbol(cola, raiz, visitados);
    }
    assert(cantidad == cola.size());
    assert(cola.empty() == (cantidad == 0));
    for (std::size_t v = 0; v < costos.size(); ++v) {
        assert(visitados[v] == presentes[v]);
        if (presentes[v]) {
            assert(cola.handle(static_cast<int>(v))->costo == costos[v]);
        } else {
            assert(cola.handle(static_cast<int>(v)) == nullptr);
        }
    }
}

void verificarCola(const BinomialHeap& cola, const std::vector<double>& costos) {
    verificarCola(cola, costos, std::vector<bool>(costos.size(), true));
}

// Extrae todos los pares y revisa orden, estructura y handles tras cada extracción.
void verificarExtracciones(BinomialHeap& cola, const std::vector<double>& costos) {
    std::vector<bool> presentes(costos.size(), true);
    double anterior = -std::numeric_limits<double>::infinity();
    for (std::size_t i = 0; i < costos.size(); ++i) {
        const auto [costo, vertice] = cola.extractMin();
        assert(vertice >= 0 && static_cast<std::size_t>(vertice) < costos.size());
        assert(presentes[vertice]);
        assert(costo == costos[vertice]);
        assert(costo >= anterior);
        anterior = costo;
        presentes[vertice] = false;
        assert(cola.size() == costos.size() - i - 1);
        verificarCola(cola, costos, presentes);
    }
    assert(cola.empty());
    assert(cola.roots() == nullptr);

    bool vaciaRechazada = false;
    try {
        cola.extractMin();
    } catch (const std::underflow_error&) {
        vaciaRechazada = true;
    }
    assert(vaciaRechazada);
}

int main() {
    const double infinito = std::numeric_limits<double>::infinity();
    const std::vector<std::vector<double>> casos = {
        {}, {0.5}, {0.8, 0.2},
        {0.9, 0.8, 0.7, 0.6, 0.5, 0.4, 0.3, 0.2},
        {0.4, 0.4, 0.1, 0.7, 0.1},
        {0.01, 0.8, 0.3, 0.6, 0.2, 0.9, 0.4, 0.7, 0.5, 0.2, 0.6, 0.3, 0.8},
        {infinito, infinito, 0.0, infinito, infinito}
    };
    for (const auto& costos : casos) {
        BinomialHeap cola(costos);
        verificarCola(cola, costos);
        verificarExtracciones(cola, costos);
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
    verificarExtracciones(incremental, costos);

    // Reutilizar los vértices extraídos debe volver a crear sus handles.
    for (int v = 0; v < 17; ++v) {
        incremental.insert(costos[v], v);
    }
    verificarCola(incremental, costos);
    verificarExtracciones(incremental, costos);

    std::cout << "Pruebas de construccion, insercion y extractMin: OK\n";
    return 0;
}
