#include "prim.h"
#include "binomial_heap.h"

#include <limits>
#include <stdexcept>

PrimResult primBinomial(const Graph& G, int r) {
    const std::size_t n = G.numVertices();
    if (r < 0 || static_cast<std::size_t>(r) >= n) {
        throw std::out_of_range("Raiz fuera de rango");
    }

    std::vector<double> costos(n, std::numeric_limits<double>::infinity());
    std::vector<int> parent(n, -1);
    costos[r] = 0.0;
    BinomialHeap Q(costos);
    PrimResult resultado;
    auto& T = resultado.aristas;
    T.reserve(n - 1);

    while (!Q.empty()) {
        const auto [c, v] = Q.extractMin();
        if (v != r) {
            if (parent[v] == -1) {
                throw std::invalid_argument("El grafo debe ser conexo");
            }
            T.push_back({parent[v], v});
            resultado.pesoTotal += c;
        }

        for (const auto& [u, w] : G.neighbors(v)) {
            if (Q.handle(u) != nullptr && w < costos[u]) {
                costos[u] = w;
                parent[u] = v;
                resultado.intercambios += Q.decreaseKey(u, w);
                ++resultado.llamadasDecreaseKey;
            }
        }
    }
    return resultado;
}
