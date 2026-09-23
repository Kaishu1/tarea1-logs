#include "prim.h"
#include "binomial_heap.h"

#include <limits>
#include <stdexcept>

PrimResult primBinomial(const Graph& grafo, int raiz) {
    const std::size_t n = grafo.numVertices();
    if (raiz < 0 || static_cast<std::size_t>(raiz) >= n) {
        throw std::out_of_range("Raiz fuera de rango");
    }

    std::vector<double> costos(n, std::numeric_limits<double>::infinity());
    std::vector<int> parent(n, -1);
    costos[raiz] = 0.0;
    BinomialHeap cola(costos);
    PrimResult resultado;
    resultado.aristas.reserve(n - 1);

    while (!cola.empty()) {
        const auto [costo, v] = cola.extractMin();
        if (v != raiz) {
            if (parent[v] == -1) {
                throw std::invalid_argument("El grafo debe ser conexo");
            }
            resultado.aristas.push_back({parent[v], v});
            resultado.pesoTotal += costo;
        }

        for (const auto& vecino : grafo.neighbors(v)) {
            const int u = vecino.vertice;
            if (cola.handle(u) != nullptr && vecino.peso < costos[u]) {
                costos[u] = vecino.peso;
                parent[u] = v;
                resultado.intercambios += cola.decreaseKey(u, vecino.peso);
                ++resultado.llamadasDecreaseKey;
            }
        }
    }
    return resultado;
}
