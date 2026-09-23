#include "binomial_heap.h"
#include "prim.h"

#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>

// Revisa forma binomial, orden de heap y handles; devuelve el tamaño del árbol.
std::size_t verificarArbol(const BinomialHeap& cola,
                          const BinomialHeap::Node* nodo,
                          std::vector<bool>& visitados) {
    assert(nodo->vertice >= 0);
    assert(static_cast<std::size_t>(nodo->vertice) < visitados.size());
    assert(!visitados[nodo->vertice]);
    visitados[nodo->vertice] = true;
    assert(cola.handle(nodo->vertice) == nodo);

    std::size_t cantidad = 1;
    int gradoEsperado = nodo->grado - 1;
    for (const auto* hijo = nodo->hijo; hijo != nullptr; hijo = hijo->hermano) {
        assert(hijo->padre == nodo);
        assert(hijo->costo >= nodo->costo);
        assert(hijo->grado == gradoEsperado);
        --gradoEsperado;
        cantidad += verificarArbol(cola, hijo, visitados);
    }
    assert(gradoEsperado == -1);
    assert(cantidad == (std::size_t{1} << nodo->grado));
    return cantidad;
}

// Revisa raíces con grados crecientes y presencia de cada vértice esperado.
void verificarCola(const BinomialHeap& cola, const std::vector<double>& costos,
                   const std::vector<bool>& presentes) {
    std::vector<bool> visitados(costos.size(), false);
    std::size_t cantidad = 0;
    int gradoAnterior = -1;
    for (const auto* raiz = cola.roots(); raiz != nullptr; raiz = raiz->hermano) {
        assert(raiz->padre == nullptr);
        assert(raiz->grado > gradoAnterior);
        gradoAnterior = raiz->grado;
        cantidad += verificarArbol(cola, raiz, visitados);
    }
    assert(cantidad == cola.size());
    assert(cola.empty() == (cantidad == 0));
    for (std::size_t v = 0; v < costos.size(); ++v) {
        assert(visitados[v] == presentes[v]);
        if (presentes[v]) {
            assert(cola.handle(static_cast<int>(v))->costo == costos[v]);
        } else {
            assert(cola.handle(static_cast<int>(v)) == nullptr);
        }
    }
}

void verificarCola(const BinomialHeap& cola, const std::vector<double>& costos) {
    verificarCola(cola, costos, std::vector<bool>(costos.size(), true));
}

// Extrae todos los pares y revisa orden, estructura y handles tras cada extracción.
void verificarExtracciones(BinomialHeap& cola, const std::vector<double>& costos) {
    std::vector<bool> presentes(costos.size(), true);
    double anterior = -std::numeric_limits<double>::infinity();
    for (std::size_t i = 0; i < costos.size(); ++i) {
        const auto [costo, vertice] = cola.extractMin();
        assert(vertice >= 0 && static_cast<std::size_t>(vertice) < costos.size());
        assert(presentes[vertice]);
        assert(costo == costos[vertice]);
        assert(costo >= anterior);
        anterior = costo;
        presentes[vertice] = false;
        assert(cola.size() == costos.size() - i - 1);
        verificarCola(cola, costos, presentes);
    }
    assert(cola.empty());
    assert(cola.roots() == nullptr);

    bool vaciaRechazada = false;
    try {
        cola.extractMin();
    } catch (const std::underflow_error&) {
        vaciaRechazada = true;
    }
    assert(vaciaRechazada);
}

// Prueba reducciones consecutivas y el número de intercambios de cada llamada.
void verificarDecreaseKey() {
    std::vector<double> costos{0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8};
    BinomialHeap cola(costos);
    struct Reduccion {
        int vertice;
        double costo;
        std::size_t intercambios;
    };
    const std::vector<Reduccion> reducciones = {
        {0, 0.05, 0},    // Reducir la raíz no requiere intercambios.
        {7, 0.75, 0},    // Sigue siendo mayor que su padre.
        {7, 0.75, 0},    // Mantener el mismo costo no cambia nada.
        {7, 0.7, 0},     // Igualar al padre tampoco requiere intercambios.
        {7, 0.01, 3},    // Sube desde profundidad tres hasta la raíz.
        {0, 0.005, 1},   // Usa el handle de un vértice desplazado.
        {7, 0.001, 1},   // Vuelve a reducir el mismo vértice tras moverse.
        {6, 0.0001, 3}
    };
    for (const auto& reduccion : reducciones) {
        const auto intercambios = cola.decreaseKey(reduccion.vertice, reduccion.costo);
        assert(intercambios == reduccion.intercambios);
        costos[reduccion.vertice] = reduccion.costo;
        verificarCola(cola, costos);
    }

    for (int vertice : {-1, 8}) {
        bool rechazado = false;
        try {
            cola.decreaseKey(vertice, 0.0);
        } catch (const std::out_of_range&) {
            rechazado = true;
        }
        assert(rechazado);
        verificarCola(cola, costos);
    }
    for (double costo : {0.9, std::numeric_limits<double>::quiet_NaN()}) {
        bool rechazado = false;
        try {
            cola.decreaseKey(0, costo);
        } catch (const std::invalid_argument&) {
            rechazado = true;
        }
        assert(rechazado);
        verificarCola(cola, costos);
    }
    verificarExtracciones(cola, costos);

    bool ausenteRechazado = false;
    try {
        cola.decreaseKey(0, 0.0);
    } catch (const std::invalid_argument&) {
        ausenteRechazado = true;
    }
    assert(ausenteRechazado);
    assert(cola.empty());

    std::vector<double> iniciales{0.0, std::numeric_limits<double>::infinity()};
    BinomialHeap inicioPrim(iniciales);
    const auto intercambios = inicioPrim.decreaseKey(1, 0.5);
    assert(intercambios == 0);
    iniciales[1] = 0.5;
    verificarCola(inicioPrim, iniciales);
    verificarExtracciones(inicioPrim, iniciales);
}

// Comprueba que el resultado cubra todos los vértices sin ciclos y tenga el peso esperado.
void verificarMST(const Graph& grafo, int raiz, const PrimResult& resultado,
                  double pesoEsperado) {
    assert(resultado.aristas.size() == grafo.numVertices() - 1);
    std::vector<bool> incluidos(grafo.numVertices(), false);
    incluidos[raiz] = true;
    double suma = 0.0;
    for (const auto& [u, v] : resultado.aristas) {
        assert(u >= 0 && static_cast<std::size_t>(u) < grafo.numVertices());
        assert(v >= 0 && static_cast<std::size_t>(v) < grafo.numVertices());
        assert(incluidos[u] && !incluidos[v]);
        bool existe = false;
        for (const auto& vecino : grafo.neighbors(u)) {
            if (vecino.vertice == v) {
                existe = true;
                suma += vecino.peso;
                break;
            }
        }
        assert(existe);
        incluidos[v] = true;
    }
    for (bool incluido : incluidos) {
        assert(incluido);
    }
    assert(std::abs(suma - resultado.pesoTotal) < 1e-12);
    assert(std::abs(resultado.pesoTotal - pesoEsperado) < 1e-12);
}

// Prueba un MST conocido, distintas raíces, pesos repetidos y un solo vértice.
void verificarPrimBinomial() {
    Graph grafo(4);
    grafo.addEdge(0, 1, 0.9);
    grafo.addEdge(0, 2, 0.5);
    grafo.addEdge(0, 3, 0.8);
    grafo.addEdge(1, 2, 0.2);
    grafo.addEdge(1, 3, 0.4);
    grafo.addEdge(2, 3, 0.6);
    // El MST usa 0--2, 2--1 y 1--3: peso 0.5 + 0.2 + 0.4 = 1.1.
    for (int raiz = 0; raiz < 4; ++raiz) {
        const auto resultado = primBinomial(grafo, raiz);
        verificarMST(grafo, raiz, resultado, 1.1);
        if (raiz == 0) {
            assert(resultado.llamadasDecreaseKey == 6);
            assert(resultado.intercambios > 0);
        }
    }

    Graph repetidos(4);
    repetidos.addEdge(0, 1, 0.5);
    repetidos.addEdge(1, 2, 0.5);
    repetidos.addEdge(2, 3, 0.5);
    repetidos.addEdge(3, 0, 0.5);
    const auto resultado = primBinomial(repetidos, 2);
    verificarMST(repetidos, 2, resultado, 1.5);
    assert(resultado.llamadasDecreaseKey == 3);

    Graph unico(1);
    const auto trivial = primBinomial(unico, 0);
    verificarMST(unico, 0, trivial, 0.0);
    assert(trivial.llamadasDecreaseKey == 0);
    assert(trivial.intercambios == 0);
}

int main() {
    const double infinito = std::numeric_limits<double>::infinity();
    const std::vector<std::vector<double>> casos = {
        {}, {0.5}, {0.8, 0.2},
        {0.9, 0.8, 0.7, 0.6, 0.5, 0.4, 0.3, 0.2},
        {0.4, 0.4, 0.1, 0.7, 0.1},
        {0.01, 0.8, 0.3, 0.6, 0.2, 0.9, 0.4, 0.7, 0.5, 0.2, 0.6, 0.3, 0.8},
        {infinito, infinito, 0.0, infinito, infinito}
    };
    for (const auto& costos : casos) {
        BinomialHeap cola(costos);
        verificarCola(cola, costos);
        verificarExtracciones(cola, costos);
    }

    BinomialHeap incremental(17);
    std::vector<double> costos;
    for (int v = 0; v < 17; ++v) {
        assert(incremental.handle(v) == nullptr);
        costos.push_back((17 - v) / 17.0);
        incremental.insert(costos.back(), v);
        verificarCola(incremental, costos);
    }

    bool duplicadoRechazado = false;
    try {
        incremental.insert(0.1, 0);
    } catch (const std::invalid_argument&) {
        duplicadoRechazado = true;
    }
    assert(duplicadoRechazado);

    bool indiceRechazado = false;
    try {
        incremental.insert(0.1, 17);
    } catch (const std::out_of_range&) {
        indiceRechazado = true;
    }
    assert(indiceRechazado);
    verificarCola(incremental, costos);
    verificarExtracciones(incremental, costos);

    // Reutilizar los vértices extraídos debe volver a crear sus handles.
    for (int v = 0; v < 17; ++v) {
        incremental.insert(costos[v], v);
    }
    verificarCola(incremental, costos);
    verificarExtracciones(incremental, costos);

    verificarDecreaseKey();
    verificarPrimBinomial();

    std::cout << "Pruebas de cola binomial y Prim binomial: OK\n";
    return 0;
}
