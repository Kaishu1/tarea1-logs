#ifndef PRIM_H
#define PRIM_H

#include "graph.h"

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

// Árbol de expansión mínima y métricas de decreaseKey obtenidos por Prim.
struct PrimResult {
    // Aristas del árbol como pares (padre, vértice).
    std::vector<std::pair<int, int>> aristas;
    // Peso total del árbol.
    double pesoTotal = 0.0;
    // Llamadas e intercambios (binomial) o cortes (Fibonacci).
    std::size_t llamadasDecreaseKey = 0;
    std::size_t intercambios = 0;
    std::size_t cortes = 0;
    // Tiempo acumulado de decreaseKey, en segundos.
    double tiempoDecreaseKey = 0.0;
};

// Valores por llamada a decreaseKey; pueden ser nanosegundos o operaciones.
struct RegistroDecreaseKey {
    // true registra tiempo; false registra operaciones estructurales.
    bool medirTiempo = false;
    std::vector<std::uint64_t> valores;
};

// Ejecuta Prim con heap binomial; recibe grafo, raíz y registro opcional, devuelve MST y métricas.
// Requiere grafo conexo y raíz válida; lanza si no se cumple.
PrimResult primBinomial(const Graph& G, int r,
                        RegistroDecreaseKey* registro = nullptr);

// Ejecuta Prim con heap Fibonacci; recibe grafo, raíz y registro opcional, devuelve MST y métricas.
// Requiere grafo conexo y raíz válida; lanza si no se cumple.
PrimResult primFibonacci(const Graph& G, int r,
                         RegistroDecreaseKey* registro = nullptr);

#endif