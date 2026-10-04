#ifndef GRAPH_H
#define GRAPH_H

#include <cstddef>
#include <cstdint>
#include <vector>

// Grafo no dirigido y con pesos, representado mediante listas de adyacencia.
class Graph {
public:
    // Vecino adyacente y peso de la arista que lo conecta.
    struct Neighbor {
        int vertice;
        double peso;
    };

    // Crea vértices numerados desde 0; recibe su cantidad.
    explicit Graph(std::size_t cantidadVertices);

    // Agrega la arista no dirigida (u, v, peso); lanza si los datos no son válidos.
    void addEdge(int u, int v, double peso);

    // Devuelve la cantidad de vértices.
    std::size_t numVertices() const;

    // Devuelve por referencia los vecinos de vertice; lanza si está fuera de rango.
    const std::vector<Neighbor>& neighbors(int vertice) const;

private:
    std::vector<std::vector<Neighbor>> adyacencia;
};

// Genera un grafo simple, conexo y con pesos en (0, 1]. Recibe V, E y la semilla aleatoria.
// Devuelve el grafo; lanza si V y E no permiten construirlo.
Graph generateConnectedGraph(std::size_t cantidadVertices, std::size_t cantidadAristas,
                            std::uint64_t semilla);

#endif
