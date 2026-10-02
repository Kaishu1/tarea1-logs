#ifndef PRIM_H
#define PRIM_H

#include "graph.h"

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

struct PrimResult {
    std::vector<std::pair<int, int>> aristas;
    double pesoTotal = 0.0;
    std::size_t llamadasDecreaseKey = 0;
    std::size_t intercambios = 0;
    std::size_t cortes = 0;
    double tiempoDecreaseKey = 0.0;
};

struct RegistroDecreaseKey {
    bool medirTiempo = false;
    std::vector<std::uint64_t> valores;
};

PrimResult primBinomial(const Graph& G, int r,
                        RegistroDecreaseKey* registro = nullptr);

PrimResult primFibonacci(const Graph& G, int r,
                         RegistroDecreaseKey* registro = nullptr);

#endif