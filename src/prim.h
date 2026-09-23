#ifndef PRIM_H
#define PRIM_H

#include "graph.h"

#include <cstddef>
#include <utility>
#include <vector>

// Aristas (padre, vértice), peso del MST y conteos de decreaseKey.
struct PrimResult {
    std::vector<std::pair<int, int>> aristas;
    double pesoTotal = 0.0;
    std::size_t llamadasDecreaseKey = 0;
    std::size_t intercambios = 0;
};

// Construye el MST del grafo desde una raíz válida usando la cola binomial.
// Requiere un grafo conexo; devuelve sus aristas, peso total y contadores.
PrimResult primBinomial(const Graph& G, int r);

#endif
