#include "graph.h"

#include <cmath>
#include <stdexcept>

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
