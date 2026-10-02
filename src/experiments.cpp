#include "experiments.h"
#include "binomial_heap.h"
#include "graph.h"
#include "prim.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <sys/resource.h>
#include <sys/utsname.h>

namespace {
constexpr int repeticiones = 10;
constexpr int puntosCurva = 100;
constexpr std::uint64_t semillaBase = 42;
constexpr double MiB = 1048576.0;
using Reloj = std::chrono::steady_clock;

struct Configuracion { int i; int j; };

// Devuelve las cinco configuraciones de una serie; el piloto conserva sus formas.
std::vector<Configuracion> configuraciones(char serie, bool piloto) {
    std::vector<Configuracion> casos;
    for (int k = 0; k < 5; ++k) {
        if (serie == 'A') casos.push_back({piloto ? 8 : 20, (piloto ? 8 : 20) + k});
        if (serie == 'B') casos.push_back({(piloto ? 7 : 18) + k, piloto ? 12 : 24});
        if (serie == 'C') casos.push_back({piloto ? 7 : 18, (piloto ? 7 : 18) + k});
        if (serie == 'D') casos.push_back({(piloto ? 7 : 14) + k, piloto ? 11 : 22});
    }
    return casos;
}

// Lee bytes desde un campo de /proc; las consultas se hacen fuera de los relojes.
std::uint64_t memoria(const std::string& archivo, const std::string& campo) {
    std::ifstream entrada(archivo);
    std::string linea;
    while (std::getline(entrada, linea)) {
        if (linea.rfind(campo + ":", 0) == 0) {
            std::istringstream datos(linea);
            std::string nombre;
            std::uint64_t kib;
            if (datos >> nombre >> kib) return kib * 1024;
        }
    }
    throw std::runtime_error("No se pudo leer " + campo + " de " + archivo);
}

long fallosMayores() {
    struct rusage uso{};
    if (getrusage(RUSAGE_SELF, &uso) != 0) {
        throw std::runtime_error("No se pudo consultar getrusage");
    }
    return uso.ru_majflt;
}

struct DatosGrafo {
    std::uint64_t huella = 14695981039346656037ULL;
    std::uint64_t capacidadBytes = 0;
};

// Identifica tamanos, orden de vecinos y pesos exactos con FNV-1a de 64 bits.
// Se recorre el grafo antes de medir Prim. La huella no es criptografica.
DatosGrafo identificar(const Graph& G, std::size_t E) {
    DatosGrafo datos;
    auto agregar = [&](std::uint64_t valor) {
        for (int byte = 0; byte < 8; ++byte) {
            datos.huella ^= (valor >> (8 * byte)) & 255;
            datos.huella *= 1099511628211ULL;
        }
    };
    agregar(G.numVertices());
    agregar(E);
    std::size_t entradas = 0;
    datos.capacidadBytes = G.numVertices() * sizeof(std::vector<Graph::Neighbor>);
    for (std::size_t u = 0; u < G.numVertices(); ++u) {
        const auto& vecinos = G.neighbors(static_cast<int>(u));
        entradas += vecinos.size();
        datos.capacidadBytes += vecinos.capacity() * sizeof(Graph::Neighbor);
        agregar(u);
        agregar(vecinos.size());
        for (const auto& vecino : vecinos) {
            std::uint64_t bits;
            static_assert(sizeof(bits) == sizeof(vecino.peso));
            std::memcpy(&bits, &vecino.peso, sizeof(bits));
            agregar(vecino.vertice);
            agregar(bits);
        }
    }
    if (entradas != 2 * E) throw std::runtime_error("Cantidad de aristas incorrecta");
    return datos;
}

void verificarResultado(const PrimResult& T, std::size_t V, std::size_t E) {
    if (T.aristas.size() != V - 1 || !std::isfinite(T.pesoTotal) ||
        T.llamadasDecreaseKey < V - 1 || T.llamadasDecreaseKey > E) {
        throw std::runtime_error("Resultado de Prim invalido");
    }
}

// Suma el registro completo y guarda 101 prefijos, desde cero hasta la ultima llamada.
// El muestreo solo reduce la salida: todas las llamadas contribuyen a las sumas.
std::uint64_t guardarCurva(std::ofstream& salida, const RegistroDecreaseKey& registro,
                          int i, int j, int repeticion, std::uint64_t semilla) {
    std::uint64_t acumulado = 0;
    std::size_t k = 0;
    for (int punto = 0; punto <= puntosCurva; ++punto) {
        const auto hasta = registro.valores.size() * punto / puntosCurva;
        while (k < hasta) acumulado += registro.valores[k++];
        salida << i << ',' << j << ',' << repeticion << ',' << semilla << ','
               << (registro.medirTiempo ? "tiempo_ns" : "intercambios") << ','
               << punto << ',' << hasta << ',' << acumulado << '\n';
    }
    return acumulado;
}

// Guarda metadatos de ejecucion junto a las mediciones de cada serie.
void guardarEntorno(std::ofstream& salida, char serie, bool piloto) {
    struct utsname sistema{};
    if (uname(&sistema) == 0) {
        salida << "Sistema: " << sistema.sysname << ' ' << sistema.release
               << "; arquitectura: " << sistema.machine << '\n';
    }
    std::ifstream cpu("/proc/cpuinfo");
    std::string linea;
    while (std::getline(cpu, linea)) {
        if (linea.rfind("model name", 0) == 0) { salida << linea << '\n'; break; }
    }
    salida << "Compilador: " << __VERSION__ << "; __cplusplus=" << __cplusplus << '\n'
           << "Serie: " << serie << "; modo: " << (piloto ? "piloto" : "oficial") << '\n'
           << "Repeticiones: 10; raiz: 0; semilla: 42 + 10000*i + 100*j + repeticion\n"
           << "Pesos: uniformes en (0,1]; generador: mt19937_64\n"
           << "Reloj: steady_clock; periodo: " << Reloj::period::num << '/'
           << Reloj::period::den << " s (no equivale a resolucion medida)\n"
           << "RAM total bytes: " << memoria("/proc/meminfo", "MemTotal") << '\n'
           << "RAM disponible inicial bytes: " << memoria("/proc/meminfo", "MemAvailable") << '\n'
           << "Registro C/D: E * sizeof(uint64_t) bytes; una pasada a la vez\n"
           << "C/D conserva un MST adicional para verificar las dos pasadas\n"
           << "Tamanos ajustados durante la ejecucion: 0\n"
           << "Pico RSS: maximo del proceso hasta cada fila, incluye generacion\n"
           << "Swap: maximo de muestras antes/despues de generacion y Prim\n"
           << "Tiempo total: incluye construccion y liberacion de la cola y auxiliares\n"
           << "Tiempo decreaseKey: incluye costo del reloj, sin escritura del registro\n"
           << "Curvas: 101 prefijos por pasada, calculados despues de Prim\n";
}

// Ejecuta una serie, imprime diez filas por configuracion y sus promedios.
void ejecutarSerie(char serie, const std::filesystem::path& carpeta, bool piloto) {
    std::filesystem::create_directories(carpeta);
    std::ofstream filas(carpeta / "repeticiones.csv");
    std::ofstream promedios(carpeta / "promedios.csv");
    std::ofstream curvas(carpeta / "curvas.csv");
    std::ofstream entorno(carpeta / "entorno.txt");
    for (auto* archivo : {&filas, &promedios, &curvas, &entorno}) {
        archivo->exceptions(std::ios::failbit | std::ios::badbit);
        *archivo << std::setprecision(17);
    }
    guardarEntorno(entorno, serie, piloto);
    filas << "modo,algoritmo,serie,i,j,V,E,repeticion,semilla,raiz,huella_grafo,"
             "tiempo_prim_ms,decrease_ms,llamadas,intercambios,peso_mst,aristas_mst,"
             "registro_bytes,adyacencia_bytes,ram_disponible_bytes,pico_proceso_bytes,"
             "swap_muestras_bytes,fallos_mayores_prim\n";
    promedios << "modo,algoritmo,serie,i,j,V,E,repeticiones,tiempo_prim_ms,decrease_ms,"
                 "llamadas,intercambios,peso_mst,intercambios_por_llamada\n";
    curvas << "i,j,repeticion,semilla,medicion,punto,llamadas,acumulado\n";
    const bool total = serie == 'A' || serie == 'B';
    const char* modo = piloto ? "piloto" : "oficial";
    std::uint64_t picoSerie = 0, swapSerie = 0;
    long fallosSerie = 0;
    std::cout << "\nSerie " << serie << " - " << modo << " - 10 repeticiones\n"
              << (total ? "Tiempo: Prim completo (ms)\n" : "Tiempo: suma de decreaseKey (ms)\n");

    for (const auto& [i, j] : configuraciones(serie, piloto)) {
        const std::size_t V = std::size_t{1} << i;
        const std::size_t E = std::size_t{1} << j;
        const std::size_t registroBytes = total ? 0 : E * sizeof(std::uint64_t);
        const std::size_t estimacion = 2 * E * sizeof(Graph::Neighbor)
            + V * (sizeof(std::vector<Graph::Neighbor>) + sizeof(BinomialHeap::Node)
                   + sizeof(double) + sizeof(int) + sizeof(BinomialHeap::Node*))
            + (total ? 1 : 2) * (V - 1) * sizeof(std::pair<int, int>) + registroBytes;
        std::cout << "\ni=" << i << ", j=" << j << ", V=" << V << ", E=" << E
                  << ", raiz=0\nEstimacion logica: " << std::fixed << std::setprecision(3)
                  << estimacion / MiB << " MiB; registro: " << registroBytes / MiB
                  << " MiB; RAM disponible: " << memoria("/proc/meminfo", "MemAvailable") / MiB
                  << " MiB\n" << std::right << std::setw(5) << "Rep" << ' ' << std::setw(12) << "Semilla"
                  << ' ' << std::setw(15) << "Tiempo ms" << ' ' << std::setw(16) << "Llamadas"
                  << ' ' << std::setw(17) << "Intercambios" << ' ' << std::setw(19) << "Peso MST" << '\n';
        double sumaTiempo = 0, sumaLlamadas = 0, sumaIntercambios = 0, sumaPeso = 0;
        double sumaPorLlamada = 0;
        for (int repeticion = 1; repeticion <= repeticiones; ++repeticion) {
            // La misma configuracion y repeticion usan la misma semilla en ambas colas/series.
            const std::uint64_t semilla = semillaBase + 10000 * i + 100 * j + repeticion;
            std::cout << std::flush;
            const auto ramDisponible = memoria("/proc/meminfo", "MemAvailable");
            auto swap = memoria("/proc/self/status", "VmSwap");
            const Graph G = generateConnectedGraph(V, E, semilla);
            const auto datos = identificar(G, E);
            swap = std::max(swap, memoria("/proc/self/status", "VmSwap"));
            PrimResult T;
            double tiempo;
            long fallos = 0;
            if (total) {
                const auto antes = fallosMayores();
                const auto inicio = Reloj::now();
                T = primBinomial(G, 0);
                const auto fin = Reloj::now();
                tiempo = std::chrono::duration<double, std::milli>(fin - inicio).count();
                fallos = fallosMayores() - antes;
                verificarResultado(T, V, E);
            } else {
                RegistroDecreaseKey registro;
                registro.valores.reserve(E);
                registro.medirTiempo = true;
                auto antes = fallosMayores();
                T = primBinomial(G, 0, &registro);
                fallos = fallosMayores() - antes;
                verificarResultado(T, V, E);
                if (registro.valores.size() != T.llamadasDecreaseKey) {
                    throw std::runtime_error("Registro de tiempos incompleto");
                }
                tiempo = guardarCurva(curvas, registro, i, j, repeticion, semilla) / 1e6;
                swap = std::max(swap, memoria("/proc/self/status", "VmSwap"));
                // Se reutiliza la reserva; la segunda pasada cuenta sin consultar el reloj.
                registro.medirTiempo = false;
                antes = fallosMayores();
                const PrimResult conteo = primBinomial(G, 0, &registro);
                fallos += fallosMayores() - antes;
                const auto intercambios = guardarCurva(curvas, registro, i, j, repeticion, semilla);
                if (registro.valores.size() != T.llamadasDecreaseKey ||
                    intercambios != T.intercambios || conteo.intercambios != T.intercambios ||
                    conteo.llamadasDecreaseKey != T.llamadasDecreaseKey ||
                    conteo.aristas != T.aristas || conteo.pesoTotal != T.pesoTotal) {
                    throw std::runtime_error("Las pasadas de tiempo y conteo no coinciden");
                }
            }
            swap = std::max(swap, memoria("/proc/self/status", "VmSwap"));
            const auto pico = memoria("/proc/self/status", "VmHWM");
            picoSerie = std::max(picoSerie, pico);
            swapSerie = std::max(swapSerie, swap);
            fallosSerie += fallos;
            filas << modo << ",binomial," << serie << ',' << i << ',' << j << ',' << V << ',' << E
                  << ',' << repeticion << ',' << semilla << ",0," << datos.huella << ',';
            if (total) filas << tiempo << ','; else filas << ',' << tiempo;
            filas << ',' << T.llamadasDecreaseKey << ',' << T.intercambios << ',' << T.pesoTotal
                  << ',' << T.aristas.size() << ',' << registroBytes << ',' << datos.capacidadBytes
                  << ',' << ramDisponible << ',' << pico << ',' << swap << ',' << fallos << '\n';
            filas.flush();
            curvas.flush();
            sumaTiempo += tiempo;
            sumaLlamadas += T.llamadasDecreaseKey;
            sumaIntercambios += T.intercambios;
            sumaPeso += T.pesoTotal;
            sumaPorLlamada += static_cast<double>(T.intercambios) / T.llamadasDecreaseKey;
            std::cout << std::setw(5) << repeticion << ' ' << std::setw(12) << semilla
                      << ' ' << std::setprecision(6) << std::setw(15) << tiempo
                      << ' ' << std::setw(16) << T.llamadasDecreaseKey << ' ' << std::setw(17) << T.intercambios
                      << ' ' << std::setprecision(10) << std::setw(19) << T.pesoTotal << '\n';
            if (swap > 0 || fallos > 0) {
                std::cout << "  Swap observado: " << std::setprecision(3) << swap / MiB
                          << " MiB; fallos mayores durante Prim: " << fallos << '\n';
            }
        }
        promedios << modo << ",binomial," << serie << ',' << i << ',' << j << ',' << V << ',' << E
                  << ',' << repeticiones << ',';
        if (total) promedios << sumaTiempo / repeticiones << ',';
        else promedios << ',' << sumaTiempo / repeticiones;
        promedios << ',' << sumaLlamadas / repeticiones << ',' << sumaIntercambios / repeticiones
                  << ',' << sumaPeso / repeticiones << ',' << sumaPorLlamada / repeticiones << '\n';
        promedios.flush();
        std::cout << std::setw(18) << "Promedio"
                  << ' ' << std::setprecision(6) << std::setw(15) << sumaTiempo / repeticiones
                  << ' ' << std::setw(16) << sumaLlamadas / repeticiones
                  << ' ' << std::setw(17) << sumaIntercambios / repeticiones
                  << ' ' << std::setprecision(10) << std::setw(19) << sumaPeso / repeticiones << '\n';
    }
    std::cout << std::setprecision(3) << "Pico RSS del proceso hasta esta serie: "
              << picoSerie / MiB << " MiB; swap maximo observado: " << swapSerie / MiB
              << " MiB; fallos mayores durante Prim: " << fallosSerie << '\n';
    std::cout << "Archivos: " << carpeta.string() << "\nEstado: 50/50 ejecuciones completas\n";
}
} // namespace

int ejecutarExperimentos(const std::string& serie, const std::string& directorio, bool piloto) {
    if (serie != "todas" && serie != "A" && serie != "B" && serie != "C" && serie != "D") {
        throw std::invalid_argument("La serie debe ser A, B, C, D o todas");
    }
    const std::string series = serie == "todas" ? "ABCD" : serie;
    // Valida todas las carpetas antes de comenzar para no pisar ejecuciones anteriores.
    for (char s : series) {
        const auto carpeta = std::filesystem::path(directorio) / std::string(1, s);
        for (const char* nombre : {"repeticiones.csv", "promedios.csv", "curvas.csv", "entorno.txt"}) {
            if (std::filesystem::exists(carpeta / nombre)) {
                throw std::runtime_error("Ya existe " + (carpeta / nombre).string()
                                         + "; elija otro directorio de salida");
            }
        }
    }
    for (char s : series) {
        ejecutarSerie(s, std::filesystem::path(directorio) / std::string(1, s), piloto);
    }
    return 0;
}
