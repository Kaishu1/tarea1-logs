#ifndef PRIM_H
#define PRIM_H

#include "graph.h"

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

// Aristas (padre, vértice), peso del MST y conteos de decreaseKey.
struct PrimResult {
    std::vector<std::pair<int, int>> aristas;
    double pesoTotal = 0.0;
    std::size_t llamadasDecreaseKey = 0;
    std::size_t intercambios = 0;
};

// Registra una cantidad por llamada: nanosegundos o intercambios, incluidos ceros.
// El llamador puede reservar espacio antes de ejecutar Prim; Prim limpia valores.
struct RegistroDecreaseKey {
    bool medirTiempo = false;
    std::vector<std::uint64_t> valores;
};

// Construye el MST del grafo desde una raíz válida usando la cola binomial.
// Requiere un grafo conexo; devuelve sus aristas, peso total y contadores.
// Sin registro no se consulta el reloj ni se guarda detalle por llamada.
PrimResult primBinomial(const Graph& G, int r, RegistroDecreaseKey* registro = nullptr);

#endif
