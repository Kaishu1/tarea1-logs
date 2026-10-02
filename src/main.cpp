#include "binomial_heap.h"
#include "fibonacci_heap.h"
#include "prim.h"
#include "graph.h"

#include <chrono>
#include <cmath>
#include <cassert>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>

// Estructura para almacenar resultados de medición de tiempo y conteo de operaciones.
struct Medicion {
    double tiempoTotal;
    double tiempoDecreaseKey;
    std::size_t llamadasDecreaseKey;
    std::size_t operaciones;
    double pesoMST;
};

// Estructura para almacenar promedios de medición de tiempo y conteo de operaciones.
struct Promedio {
    double tiempoTotal = 0.0;
    double tiempoDecreaseKey = 0.0;
    double llamadasDecreaseKey = 0.0;
    double operaciones = 0.0;
    double pesoMST = 0.0;
};

struct Promedios {
    Promedio binomial;
    Promedio fibonacci;
};

Medicion ejecutarUnaVez(const Graph& G, bool usarFibonacci) {
    const auto inicio = std::chrono::steady_clock::now();
    PrimResult resultado;

    if(usarFibonacci) {
        resultado = primFibonacci(G, 0);
    } else {
        resultado = primBinomial(G, 0);
    }

    const auto fin = std::chrono::steady_clock::now();
    const double tiempoTotal = std::chrono::duration<double>(fin - inicio).count();

    Medicion medicion;
    medicion.tiempoTotal = tiempoTotal;
    medicion.tiempoDecreaseKey = resultado.tiempoDecreaseKey;
    medicion.llamadasDecreaseKey = resultado.llamadasDecreaseKey;
    medicion.pesoMST = resultado.pesoTotal;

    if (usarFibonacci) {
        medicion.operaciones = resultado.cortes;
    } else {
        medicion.operaciones = resultado.intercambios;
    }

    return medicion;
}

// Revisa forma binomial, orden de heap y handles; devuelve el tamaño del árbol.
std::size_t verificarArbol(const BinomialHeap& cola,
                          const BinomialHeap::Node* nodo,
                          std::vector<bool>& visitados) {
    assert(nodo->content.vertice >= 0);
    assert(static_cast<std::size_t>(nodo->content.vertice) < visitados.size());
    assert(!visitados[nodo->content.vertice]);
    visitados[nodo->content.vertice] = true;
    assert(cola.handle(nodo->content.vertice) == nodo);

    std::size_t cantidad = 1;
    int gradoEsperado = nodo->grado - 1;
    for (const auto* hijo = nodo->hijo; hijo != nullptr; hijo = hijo->hermano) {
        assert(hijo->parent == nodo);
        assert(hijo->content.key >= nodo->content.key);
        assert(hijo->grado == gradoEsperado);
        --gradoEsperado;
        cantidad += verificarArbol(cola, hijo, visitados);
    }
    assert(gradoEsperado == -1);
    assert(cantidad == (std::size_t{1} << nodo->grado));
    return cantidad;
}

// Revisa raíces con grados crecientes y presencia de cada vértice esperado.
void verificarCola(const BinomialHeap& cola, const std::vector<double>& costos) {
    std::vector<bool> visitados(costos.size(), false);
    std::size_t cantidad = 0;
    int gradoAnterior = -1;
    for (const auto* raiz = cola.roots(); raiz != nullptr; raiz = raiz->hermano) {
        assert(raiz->parent == nullptr);
        assert(raiz->grado > gradoAnterior);
        gradoAnterior = raiz->grado;
        cantidad += verificarArbol(cola, raiz, visitados);
    }
    assert(cantidad == cola.size());
    assert(cantidad == costos.size());
    assert(cola.empty() == costos.empty());
    for (std::size_t v = 0; v < costos.size(); ++v) {
        assert(visitados[v]);
        assert(cola.handle(static_cast<int>(v))->content.key == costos[v]);
    }
}

Promedios ejecutarPromedio(std::size_t V,
                           std::size_t E,
                           std::uint64_t semillaBase) {
    Promedios promedios;

    for (std::size_t repeticion = 0; repeticion < 10; ++repeticion) {
        const std::uint64_t semilla = semillaBase + repeticion;

        Graph G = generateConnectedGraph(V, E, semilla);

        const Medicion binomial = ejecutarUnaVez(G, false);
        const Medicion fibonacci = ejecutarUnaVez(G, true);

        if (std::abs(binomial.pesoMST - fibonacci.pesoMST) > 1e-9) {
            throw std::runtime_error(
                "Binomial y Fibonacci produjeron MST con pesos distintos");
        }

        promedios.binomial.tiempoTotal += binomial.tiempoTotal;
        promedios.binomial.tiempoDecreaseKey +=
            binomial.tiempoDecreaseKey;
        promedios.binomial.llamadasDecreaseKey +=
            binomial.llamadasDecreaseKey;
        promedios.binomial.operaciones +=
            binomial.operaciones;
        promedios.binomial.pesoMST +=
            binomial.pesoMST;

        promedios.fibonacci.tiempoTotal += fibonacci.tiempoTotal;
        promedios.fibonacci.tiempoDecreaseKey +=
            fibonacci.tiempoDecreaseKey;
        promedios.fibonacci.llamadasDecreaseKey +=
            fibonacci.llamadasDecreaseKey;
        promedios.fibonacci.operaciones +=
            fibonacci.operaciones;
        promedios.fibonacci.pesoMST +=
            fibonacci.pesoMST;
    }

    promedios.binomial.tiempoTotal /= 10.0;
    promedios.binomial.tiempoDecreaseKey /= 10.0;
    promedios.binomial.llamadasDecreaseKey /= 10.0;
    promedios.binomial.operaciones /= 10.0;
    promedios.binomial.pesoMST /= 10.0;

    promedios.fibonacci.tiempoTotal /= 10.0;
    promedios.fibonacci.tiempoDecreaseKey /= 10.0;
    promedios.fibonacci.llamadasDecreaseKey /= 10.0;
    promedios.fibonacci.operaciones /= 10.0;
    promedios.fibonacci.pesoMST /= 10.0;

    return promedios;
}

int main() {
    const std::size_t V = 20;
    const std::size_t E = 40;
    const std::uint64_t semillaBase = 42;

    const Promedios resultados =
        ejecutarPromedio(V, E, semillaBase);

    const Promedio& binomial = resultados.binomial;
    const Promedio& fibonacci = resultados.fibonacci;

    std::cout << "Promedio Binomial (10 repeticiones):\n";
    std::cout << "  Tiempo total: "
              << binomial.tiempoTotal << " s\n";
    std::cout << "  Tiempo decreaseKey: "
              << binomial.tiempoDecreaseKey << " s\n";
    std::cout << "  Llamadas decreaseKey: "
              << binomial.llamadasDecreaseKey << "\n";
    std::cout << "  Intercambios: "
              << binomial.operaciones << "\n";
    std::cout << "  Peso MST: "
              << binomial.pesoMST << "\n\n";

    std::cout << "Promedio Fibonacci (10 repeticiones):\n";
    std::cout << "  Tiempo total: "
              << fibonacci.tiempoTotal << " s\n";
    std::cout << "  Tiempo decreaseKey: "
              << fibonacci.tiempoDecreaseKey << " s\n";
    std::cout << "  Llamadas decreaseKey: "
              << fibonacci.llamadasDecreaseKey << "\n";
    std::cout << "  Cortes: "
              << fibonacci.operaciones << "\n";
    std::cout << "  Peso MST: "
              << fibonacci.pesoMST << "\n";

    return 0;
}