#include "graph.h"

#include <cmath>
#include <limits>
#include <random>
#include <stdexcept>
#include <unordered_set>
#include <utility>

Graph::Graph(std::size_t cantidadVertices) : adyacencia(cantidadVertices) {}

void Graph::addEdge(int u, int v, double peso) {
    if (u < 0 || v < 0 || static_cast<std::size_t>(u) >= adyacencia.size() ||
        static_cast<std::size_t>(v) >= adyacencia.size()) {
        throw std::out_of_range("Vertice fuera de rango");
    }
    if (u == v || !std::isfinite(peso) || peso <= 0.0) {
        throw std::invalid_argument("La arista debe unir vertices distintos con peso positivo finito");
    }
    adyacencia[u].push_back({v, peso});
    adyacencia[v].push_back({u, peso});
}

std::size_t Graph::numVertices() const {
    return adyacencia.size();
}

const std::vector<Graph::Neighbor>& Graph::neighbors(int vertice) const {
    return adyacencia.at(vertice);
}

Graph generateConnectedGraph(std::size_t cantidadVertices, std::size_t cantidadAristas,
                            std::uint64_t semilla) {
    if (cantidadVertices == 0 ||
        cantidadVertices > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
        throw std::invalid_argument("La cantidad de vertices debe ser positiva y caber en int");
    }
    const std::uint64_t n = cantidadVertices;
    const std::uint64_t maxAristas = n * (n - 1) / 2;
    if (cantidadAristas < cantidadVertices - 1 || cantidadAristas > maxAristas) {
        throw std::invalid_argument("Cantidad de aristas invalida para un grafo simple y conexo");
    }

    Graph grafo(cantidadVertices);
    std::mt19937_64 generador(semilla);
    std::uniform_real_distribution<double> pesos(0.0, 1.0);
    std::unordered_set<std::uint64_t> aristas;
    aristas.reserve(cantidadAristas);

    // Ordena los extremos para que (u, v) y (v, u) tengan el mismo identificador.
    auto agregarArista = [&](int u, int v) {
        if (u > v) {
            std::swap(u, v);
        }
        const std::uint64_t id = static_cast<std::uint64_t>(u) * n + v;
        if (aristas.insert(id).second) {
            double peso;
            do {
                peso = 1.0 - pesos(generador);
            } while (peso == 0.0);
            grafo.addEdge(u, v, peso);
        }
    };

    // Primero conecta cada vértice con uno anterior: quedan V - 1 aristas.
    for (int v = 1; v < static_cast<int>(cantidadVertices); ++v) {
        std::uniform_int_distribution<int> anterior(0, v - 1);
        agregarArista(v, anterior(generador));
    }

    // Completa las E aristas descartando lazos y pares que ya estén presentes.
    std::uniform_int_distribution<int> vertice(0, static_cast<int>(cantidadVertices) - 1);
    while (aristas.size() < cantidadAristas) {
        const int u = vertice(generador);
        const int v = vertice(generador);
        if (u != v) {
            agregarArista(u, v);
        }
    }
    return grafo;
}
