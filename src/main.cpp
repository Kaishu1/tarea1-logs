#include "binomial_heap.h"
#include "graph.h"

#include <cassert>
#include <cmath>
#include <cstdint>
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

// Revisa tamaños, aristas en ambos sentidos, pesos y conectividad del grafo.
void verificarGrafo(const Graph& G, std::size_t cantidadVertices,
                    std::size_t cantidadAristas) {
    assert(G.numVertices() == cantidadVertices);
    assert(cantidadVertices > 0);
    std::size_t entradas = 0;

    for (int u = 0; u < static_cast<int>(cantidadVertices); ++u) {
        std::vector<bool> vecinosVistos(cantidadVertices, false);
        for (const auto& [v, peso] : G.neighbors(u)) {
            assert(v >= 0 && static_cast<std::size_t>(v) < cantidadVertices);
            assert(u != v);
            assert(!vecinosVistos[v]);
            vecinosVistos[v] = true;
            assert(std::isfinite(peso) && peso > 0.0 && peso <= 1.0);
            ++entradas;

            bool inversaEncontrada = false;
            for (const auto& inversa : G.neighbors(v)) {
                if (inversa.vertice == u) {
                    assert(inversa.peso == peso);
                    inversaEncontrada = true;
                    break;
                }
            }
            assert(inversaEncontrada);
        }
    }
    // Cada arista no dirigida aparece en las listas de sus dos extremos.
    assert(entradas == 2 * cantidadAristas);

    std::vector<bool> visitados(cantidadVertices, false);
    std::vector<int> pendientes{0};
    visitados[0] = true;
    for (std::size_t i = 0; i < pendientes.size(); ++i) {
        for (const auto& vecino : G.neighbors(pendientes[i])) {
            if (!visitados[vecino.vertice]) {
                visitados[vecino.vertice] = true;
                pendientes.push_back(vecino.vertice);
            }
        }
    }
    assert(pendientes.size() == cantidadVertices);
}

// Prueba grafos pequeños y que repetir tamaños y semilla produzca el mismo grafo.
void verificarGenerador() {
    struct Caso {
        std::size_t vertices;
        std::size_t aristas;
    };
    const std::vector<Caso> casos = {
        {1, 0}, {2, 1},
        {8, 7},    // Solo el árbol inicial.
        {8, 12},   // Árbol más aristas adicionales.
        {8, 28},   // Grafo completo: todos los pares distintos.
        {64, 256}
    };
    for (std::uint64_t semilla : {0ULL, 1ULL, 42ULL}) {
        for (const auto& caso : casos) {
            const Graph G = generateConnectedGraph(caso.vertices, caso.aristas, semilla);
            verificarGrafo(G, caso.vertices, caso.aristas);

            const Graph repetido = generateConnectedGraph(caso.vertices, caso.aristas, semilla);
            assert(G.numVertices() == repetido.numVertices());
            for (int v = 0; v < static_cast<int>(caso.vertices); ++v) {
                const auto& vecinos = G.neighbors(v);
                const auto& mismos = repetido.neighbors(v);
                assert(vecinos.size() == mismos.size());
                for (std::size_t i = 0; i < vecinos.size(); ++i) {
                    assert(vecinos[i].vertice == mismos[i].vertice);
                    assert(vecinos[i].peso == mismos[i].peso);
                }
            }
        }
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

    verificarGenerador();
    std::cout << "Pruebas del generador: conectividad, aristas, duplicados, pesos y reproducibilidad OK\n";
    return 0;
}
