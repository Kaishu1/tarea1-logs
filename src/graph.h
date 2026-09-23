#ifndef GRAPH_H
#define GRAPH_H

#include <cstddef>
#include <vector>

// Grafo no dirigido y con pesos, representado mediante listas de adyacencia.
class Graph {
public:
    struct Neighbor {
        int vertice;
        double peso;
    };

    // Crea vértices en [0, cantidadVertices), inicialmente sin aristas.
    explicit Graph(std::size_t cantidadVertices);

    // Agrega una arista en ambos sentidos; el llamador debe evitar duplicados.
    // Requiere extremos válidos y distintos, y un peso finito mayor que cero.
    void addEdge(int u, int v, double peso);

    std::size_t numVertices() const;

    // Devuelve los vecinos y pesos del vértice indicado, sin copiar la lista.
    const std::vector<Neighbor>& neighbors(int vertice) const;

private:
    std::vector<std::vector<Neighbor>> adyacencia;
};

#endif
