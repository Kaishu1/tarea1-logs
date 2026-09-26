#include "binomial_heap.h"
#include "graph.h"
#include "prim.h"

#include <cassert>
#include <cmath>
#include <cstdint>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <new>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <sys/utsname.h>

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

// Revisa tamaños, aristas en ambos sentidos, pesos y conectividad del grafo.
void verificarGrafo(const Graph& G, std::size_t cantidadVertices,
                    std::size_t cantidadAristas) {
    assert(G.numVertices() == cantidadVertices);
    assert(cantidadVertices > 0);
    std::size_t entradas = 0;

    for (int u = 0; u < static_cast<int>(cantidadVertices); ++u) {
        std::vector<bool> vecinosVistos(cantidadVertices, false);
        for (const auto& [v, peso] : G.neighbors(u)) {
            assert(v >= 0 && static_cast<std::size_t>(v) < cantidadVertices);
            assert(u != v);
            assert(!vecinosVistos[v]);
            vecinosVistos[v] = true;
            assert(std::isfinite(peso) && peso > 0.0 && peso <= 1.0);
            ++entradas;

            bool inversaEncontrada = false;
            for (const auto& inversa : G.neighbors(v)) {
                if (inversa.vertice == u) {
                    assert(inversa.peso == peso);
                    inversaEncontrada = true;
                    break;
                }
            }
            assert(inversaEncontrada);
        }
    }
    // Cada arista no dirigida aparece en las listas de sus dos extremos.
    assert(entradas == 2 * cantidadAristas);

    std::vector<bool> visitados(cantidadVertices, false);
    std::vector<int> pendientes{0};
    visitados[0] = true;
    for (std::size_t i = 0; i < pendientes.size(); ++i) {
        for (const auto& vecino : G.neighbors(pendientes[i])) {
            if (!visitados[vecino.vertice]) {
                visitados[vecino.vertice] = true;
                pendientes.push_back(vecino.vertice);
            }
        }
    }
    assert(pendientes.size() == cantidadVertices);
}

// Prueba grafos pequeños y que repetir tamaños y semilla produzca el mismo grafo.
void verificarGenerador() {
    struct Caso {
        std::size_t vertices;
        std::size_t aristas;
    };
    const std::vector<Caso> casos = {
        {1, 0}, {2, 1},
        {8, 7},    // Solo el árbol inicial.
        {8, 12},   // Árbol más aristas adicionales.
        {8, 28},   // Grafo completo: todos los pares distintos.
        {64, 256}
    };
    for (std::uint64_t semilla : {0ULL, 1ULL, 42ULL}) {
        for (const auto& caso : casos) {
            const Graph G = generateConnectedGraph(caso.vertices, caso.aristas, semilla);
            verificarGrafo(G, caso.vertices, caso.aristas);

            const Graph repetido = generateConnectedGraph(caso.vertices, caso.aristas, semilla);
            assert(G.numVertices() == repetido.numVertices());
            for (int v = 0; v < static_cast<int>(caso.vertices); ++v) {
                const auto& vecinos = G.neighbors(v);
                const auto& mismos = repetido.neighbors(v);
                assert(vecinos.size() == mismos.size());
                for (std::size_t i = 0; i < vecinos.size(); ++i) {
                    assert(vecinos[i].vertice == mismos[i].vertice);
                    assert(vecinos[i].peso == mismos[i].peso);
                }
            }
        }
    }
}

// Lee un campo de memoria de /proc expresado en KiB y lo devuelve en bytes.
std::uint64_t leerMemoria(const std::string& archivo, const std::string& campo) {
    std::ifstream entrada(archivo);
    std::string linea;
    while (std::getline(entrada, linea)) {
        if (linea.rfind(campo + ":", 0) == 0) {
            std::istringstream datos(linea);
            std::string nombre;
            std::uint64_t kib;
            if (datos >> nombre >> kib) {
                return kib * 1024;
            }
        }
    }
    throw std::runtime_error("No se pudo leer " + campo + " desde " + archivo);
}

// Imprime una cantidad de memoria en bytes y MiB.
void imprimirMemoria(const std::string& nombre, std::uint64_t bytes) {
    std::cout << std::left << std::setw(32) << nombre
              << std::right << std::setw(16) << bytes
              << std::setw(14) << std::fixed << std::setprecision(3)
              << static_cast<double>(bytes) / (1024.0 * 1024.0) << '\n';
}

// Memoria residente, pico y swap de una muestra del proceso, en bytes.
struct MemoriaProceso {
    std::uint64_t residente;
    std::uint64_t pico;
    std::uint64_t swap;
};

// Consulta /proc, imprime los valores de la etapa y devuelve la muestra.
MemoriaProceso mostrarMemoriaProceso(const std::string& etapa) {
    const MemoriaProceso memoria{
        leerMemoria("/proc/self/status", "VmRSS"),
        leerMemoria("/proc/self/status", "VmHWM"),
        leerMemoria("/proc/self/status", "VmSwap")
    };
    std::cout << '\n' << etapa << '\n';
    imprimirMemoria("RAM residente del proceso", memoria.residente);
    imprimirMemoria("Pico residente del proceso", memoria.pico);
    imprimirMemoria("Swap del proceso", memoria.swap);
    return memoria;
}

// Proyecta el almacenamiento de Prim para otros tamaños; no genera esos grafos.
void mostrarProyecciones(std::uint64_t ramDisponible) {
    struct Caso { const char* nombre; int i; int j; };
    const Caso casos[] = {{"A: mayor E", 20, 24}, {"B: mayor V", 22, 24},
                          {"C/D: mayor caso", 18, 22}};
    std::cout << "\nProyecciones de memoria (casos no ejecutados):\n"
              << std::left << std::setw(20) << "Caso" << std::right
              << std::setw(4) << "i" << std::setw(4) << "j"
              << std::setw(16) << "Logico MiB" << std::setw(18) << "Capacidad MiB"
              << std::setw(16) << "% RAM disp." << '\n';
    for (const auto& caso : casos) {
        const std::uint64_t V = std::uint64_t{1} << caso.i;
        const std::uint64_t E = std::uint64_t{1} << caso.j;
        const std::uint64_t entradas = 2 * E * sizeof(Graph::Neighbor);
        const std::uint64_t resto = V * (sizeof(std::vector<Graph::Neighbor>)
            + sizeof(BinomialHeap::Node) + sizeof(double) + sizeof(int)
            + sizeof(BinomialHeap::Node*)) + (V - 1) * sizeof(std::pair<int, int>);
        std::cout << std::left << std::setw(20) << caso.nombre << std::right
                  << std::setw(4) << caso.i << std::setw(4) << caso.j
                  << std::setw(16) << (entradas + resto) / (1024.0 * 1024.0)
                  << std::setw(18) << (2 * entradas + resto) / (1024.0 * 1024.0);
        if (ramDisponible > 0) {
            std::cout << std::setw(16) << 100.0 * (2 * entradas + resto) / ramDisponible;
        } else {
            std::cout << std::setw(16) << "N/D";
        }
        std::cout << '\n';
    }
    std::cout << "Capacidad: modelo con 4E espacios de adyacencia; porcentaje sobre ese modelo.\n"
              << "Excluye estructuras temporales y asignador; no equivale al pico residente.\n";
}

// Estima y comprueba el caso de memoria de la sección 6.2; devuelve cero si termina.
int ejecutarMemoria() {
    const std::size_t V = std::size_t{1} << 15;
    const std::size_t E = std::size_t{1} << 20;
    const std::uint64_t semilla = 42;
    const auto ramTotal = leerMemoria("/proc/meminfo", "MemTotal");
    const auto ramDisponible = leerMemoria("/proc/meminfo", "MemAvailable");

    const std::size_t entradasBytes = 2 * E * sizeof(Graph::Neighbor);
    const std::size_t cabeceras = V * sizeof(std::vector<Graph::Neighbor>);
    const std::size_t nodos = V * sizeof(BinomialHeap::Node);
    const std::size_t costos = V * sizeof(double);
    const std::size_t parent = V * sizeof(int);
    const std::size_t handles = V * sizeof(BinomialHeap::Node*);
    const std::size_t resultado = (V - 1) * sizeof(std::pair<int, int>);
    const std::size_t total = entradasBytes + cabeceras + nodos + costos
        + parent + handles + resultado;

    std::cout << "Consumo de memoria - Prim binomial\n";
    const std::time_t ahora = std::time(nullptr);
    const std::tm* fecha = std::localtime(&ahora);
    if (fecha != nullptr) {
        std::cout << "Fecha local: " << std::put_time(fecha, "%Y-%m-%d %H:%M:%S %z") << '\n';
    }
    struct utsname sistema{};
    if (uname(&sistema) == 0) {
        std::cout << "Sistema: " << sistema.sysname << ' ' << sistema.release
                  << "; arquitectura: " << sistema.machine << '\n';
    }
    std::cout << "Compilador: " << __VERSION__ << "; __cplusplus=" << __cplusplus << '\n'
              << "V=2^15=" << V << "; E=2^20=" << E
              << "; semilla=" << semilla << "; raiz=0\n"
              << "Pesos: uniformes en (0, 1]. Unidad: 1 MiB = 1048576 bytes.\n";

    std::cout << "\nTamanos de tipos (incluyen alineacion):\n";
    struct Tipo { const char* nombre; std::size_t bytes; };
    const Tipo tipos[] = {
        {"int", sizeof(int)}, {"double", sizeof(double)},
        {"Node*", sizeof(BinomialHeap::Node*)},
        {"Graph::Neighbor", sizeof(Graph::Neighbor)},
        {"BinomialHeap::Content", sizeof(BinomialHeap::Content)},
        {"BinomialHeap::Node", sizeof(BinomialHeap::Node)},
        {"vector<Neighbor> (cabecera)", sizeof(std::vector<Graph::Neighbor>)},
        {"pair<int,int> (arista MST)", sizeof(std::pair<int, int>)}
    };
    for (const auto& tipo : tipos) {
        std::cout << std::left << std::setw(32) << tipo.nombre
                  << std::right << std::setw(8) << tipo.bytes << " bytes\n";
    }
    std::cout << "Punteros por nodo: 3 (parent, hijo, hermano).\n";

    std::cout << "\nEstimacion analitica:\n"
              << "Adyacencia: 2E * sizeof(Neighbor) + V * sizeof(vector<Neighbor>).\n"
              << "Cola: V * sizeof(Node).\n"
              << "Auxiliares: V * (sizeof(double) + sizeof(int) + sizeof(Node*)).\n"
              << "Resultado: (V - 1) * sizeof(pair<int,int>).\n\n";
    std::cout << std::left << std::setw(32) << "Componente"
              << std::right << std::setw(16) << "Bytes" << std::setw(14) << "MiB" << '\n';
    imprimirMemoria("Entradas de adyacencia (2E)", entradasBytes);
    imprimirMemoria("Cabeceras de listas (V)", cabeceras);
    imprimirMemoria("Nodos binomiales (V)", nodos);
    imprimirMemoria("Arreglo costos", costos);
    imprimirMemoria("Arreglo parent", parent);
    imprimirMemoria("Arreglo handles", handles);
    imprimirMemoria("Subtotal solicitado", total - resultado);
    imprimirMemoria("Aristas del MST", resultado);
    imprimirMemoria("Total estimado con resultado", total);

    std::cout << "\nRAM del entorno Linux al inicio:\n";
    imprimirMemoria("RAM total", ramTotal);
    imprimirMemoria("RAM disponible", ramDisponible);
    if (ramDisponible > 0) {
        std::cout << "Estimacion / RAM disponible: "
                  << 100.0 * static_cast<double>(total) / ramDisponible << " %\n";
    }
    std::cout << "Excluidos de la estimacion: capacidad sobrante, temporales,\n"
              << "objetos de tamano constante y administracion de memoria.\n";
    std::cout << "\nMediciones del proceso (bytes y MiB):\n";
    const auto antes = mostrarMemoriaProceso("Antes de generar el grafo:");
    std::cout << std::flush;

    const Graph G = generateConnectedGraph(V, E, semilla);
    std::size_t entradas = 0;
    std::size_t capacidad = 0;
    for (std::size_t v = 0; v < V; ++v) {
        entradas += G.neighbors(static_cast<int>(v)).size();
        capacidad += G.neighbors(static_cast<int>(v)).capacity();
    }
    if (entradas != 2 * E) {
        throw std::runtime_error("Cantidad de aristas incorrecta");
    }
    const std::size_t adyacenciaReservada = capacidad * sizeof(Graph::Neighbor) + cabeceras;
    std::cout << "\nEntradas de adyacencia: " << entradas
              << "; espacios reservados: " << capacidad << '\n';
    imprimirMemoria("Adyacencia con capacidad real", adyacenciaReservada);
    imprimirMemoria("Estimacion con capacidad real", total - entradasBytes - cabeceras
                    + adyacenciaReservada);
    const auto generado = mostrarMemoriaProceso("Despues de generar el grafo:");

    const PrimResult T = primBinomial(G, 0);
    if (T.aristas.size() != V - 1 || !std::isfinite(T.pesoTotal)) {
        throw std::runtime_error("Resultado de Prim invalido");
    }
    std::cout << "\nAristas del MST: " << T.aristas.size()
              << "; peso total: " << std::setprecision(10) << T.pesoTotal << '\n';
    const auto terminado = mostrarMemoriaProceso("Despues de ejecutar Prim:");
    if (ramDisponible > 0) {
        std::cout << "Pico residente / RAM disponible inicial: "
                  << 100.0 * static_cast<double>(terminado.pico) / ramDisponible << " %\n";
    }
    std::cout << "Alcance del pico: generacion, Prim y programa de medicion.\n";

    const bool swapObservado = antes.swap > 0 || generado.swap > 0 || terminado.swap > 0;
    std::cout << "Estado de ejecucion: OK\n"
              << "Ajustes automaticos de tamanos: 0\n"
              << "Swap observado (3 muestras): " << (swapObservado ? "si" : "no") << '\n';
    mostrarProyecciones(ramDisponible);
    return 0;
}

// Ejecuta las pruebas pequenas de la cola y del generador; devuelve cero si pasan.
int ejecutarPruebas() {
    const double infinito = std::numeric_limits<double>::infinity();
    const std::vector<std::vector<double>> casos = { // datitos de prueba
        {}, {0.5}, {0.8, 0.2},
        {0.9, 0.8, 0.7, 0.6, 0.5, 0.4, 0.3, 0.2},
        {0.4, 0.4, 0.1, 0.7, 0.1},
        {infinito, infinito, 0.0, infinito, infinito}
    };
    for (const auto& costos : casos) {
        BinomialHeap cola(costos);
        verificarCola(cola, costos);
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

    std::cout << "Pruebas de construccion e insercion: OK\n";

    verificarGenerador();
    std::cout << "Pruebas del generador: conectividad, aristas, duplicados, pesos y reproducibilidad OK\n";
    return 0;
}

// Selecciona pruebas o memoria desde la terminal, sin modificar el codigo.
int main(int argc, char* argv[]) {
    try {
        if (argc == 1 || (argc == 2 && std::string(argv[1]) == "--pruebas")) {
            return ejecutarPruebas();
        }
        if (argc == 2 && std::string(argv[1]) == "--memoria") {
            return ejecutarMemoria();
        }
        const bool ayuda = argc == 2 && (std::string(argv[1]) == "--ayuda" ||
                                         std::string(argv[1]) == "--help");
        if (!ayuda) {
            std::cerr << "Opcion no reconocida.\n";
        }
        std::cout << "Uso: " << argv[0] << " [--pruebas | --memoria | --ayuda]\n"
                  << "  --pruebas  Ejecuta las pruebas pequenas (opcion predeterminada).\n"
                  << "  --memoria  Consumo de memoria de la seccion 6.2.\n"
                  << "             Ejecuta V=2^15, E=2^20, semilla=42; consulta RAM y pico.\n"
                  << "             Muestra tablas y proyecciones; requiere Linux con /proc.\n";
        return ayuda ? 0 : 1;
    } catch (const std::bad_alloc&) {
        std::cerr << "Error de memoria: no se pudo completar una reserva.\n"
                  << "Estado de ejecucion: incompleta\n";
        return 1;
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }
}
