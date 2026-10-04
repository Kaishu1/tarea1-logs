#ifndef PRIM_H
#define PRIM_H

#include "graph.h"

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

// Estructura que contiene el resultado de Prim, incluyendo el peso total del árbol, las aristas del árbol y estadísticas de decreaseKey.
struct PrimResult {
    std::vector<std::pair<int, int>> aristas;
    double pesoTotal = 0.0;
    std::size_t llamadasDecreaseKey = 0;
    std::size_t intercambios = 0;
    std::size_t cortes = 0;
    double tiempoDecreaseKey = 0.0;
};

// Estructura que permite registrar información de decreaseKey durante la ejecución de Prim, incluyendo si se mide tiempo o cantidad de operaciones, y los valores registrados.
struct RegistroDecreaseKey {
    bool medirTiempo = false;
    std::vector<std::uint64_t> valores;
};

// Ejecuta Prim con la cola indicada y devuelve el resultado.
PrimResult primBinomial(const Graph& G, int r,
                        RegistroDecreaseKey* registro = nullptr);

PrimResult primFibonacci(const Graph& G, int r,
                         RegistroDecreaseKey* registro = nullptr);

#endif