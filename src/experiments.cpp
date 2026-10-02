#include "experiments.h"
#include "binomial_heap.h"
#include "fibonacci_heap.h"
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

struct Configuracion {
    int i;
    int j;
};

struct ResultadoExperimento {
    PrimResult prim;
    double tiempoTotalMs = 0.0;
    double tiempoDecreaseKeyMs = 0.0;
    std::uint64_t memoriaRegistro = 0;
    std::uint64_t capacidadAdyacencia = 0;
    std::uint64_t picoProceso = 0;
    std::uint64_t swap = 0;
    long fallosMayores = 0;
};

// Devuelve las cinco configuraciones de cada serie.
std::vector<Configuracion> configuraciones(char serie, bool piloto) {
    std::vector<Configuracion> casos;

    for (int k = 0; k < 5; ++k) {
        if (serie == 'A') {
            casos.push_back({
                piloto ? 8 : 20,
                (piloto ? 8 : 20) + k
            });
        }

        if (serie == 'B') {
            casos.push_back({
                (piloto ? 7 : 18) + k,
                piloto ? 12 : 24
            });
        }

        if (serie == 'C') {
            casos.push_back({
                piloto ? 7 : 18,
                (piloto ? 7 : 18) + k
            });
        }

        if (serie == 'D') {
            casos.push_back({
                (piloto ? 7 : 14) + k,
                piloto ? 11 : 22
            });
        }
    }

    return casos;
}

// Lee bytes desde /proc.
std::uint64_t memoria(const std::string& archivo,
                      const std::string& campo) {
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

    throw std::runtime_error(
        "No se pudo leer " + campo + " de " + archivo);
}

long fallosMayores() {
    struct rusage uso{};

    if (getrusage(RUSAGE_SELF, &uso) != 0) {
        throw std::runtime_error(
            "No se pudo consultar getrusage");
    }

    return uso.ru_majflt;
}

struct DatosGrafo {
    std::uint64_t huella = 14695981039346656037ULL;
    std::uint64_t capacidadBytes = 0;
};

// Identifica el grafo y calcula la memoria de sus listas.
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

    datos.capacidadBytes =
        G.numVertices() * sizeof(std::vector<Graph::Neighbor>);

    for (std::size_t u = 0; u < G.numVertices(); ++u) {
        const auto& vecinos =
            G.neighbors(static_cast<int>(u));

        entradas += vecinos.size();

        datos.capacidadBytes +=
            vecinos.capacity() * sizeof(Graph::Neighbor);

        agregar(u);
        agregar(vecinos.size());

        for (const auto& vecino : vecinos) {
            std::uint64_t bits;

            static_assert(sizeof(bits) == sizeof(vecino.peso));

            std::memcpy(
                &bits,
                &vecino.peso,
                sizeof(bits)
            );

            agregar(vecino.vertice);
            agregar(bits);
        }
    }

    if (entradas != 2 * E) {
        throw std::runtime_error(
            "Cantidad de aristas incorrecta");
    }

    return datos;
}

void verificarResultado(const PrimResult& T,
                        std::size_t V) {
    if (T.aristas.size() != V - 1 ||
        !std::isfinite(T.pesoTotal) ||
        T.llamadasDecreaseKey < V - 1) {
        throw std::runtime_error(
            "Resultado de Prim invalido");
    }
}

// Guarda una curva acumulada.
// Para tiempo: nanosegundos.
// Para operaciones: intercambios o cortes.
std::uint64_t guardarCurva(
    std::ofstream& salida,
    const RegistroDecreaseKey& registro,
    const std::string& algoritmo,
    int i,
    int j,
    int repeticion,
    std::uint64_t semilla) {

    std::uint64_t acumulado = 0;
    std::size_t k = 0;

    for (int punto = 0; punto <= puntosCurva; ++punto) {
        const auto hasta =
            registro.valores.size() * punto / puntosCurva;

        while (k < hasta) {
            acumulado += registro.valores[k++];
        }

        salida << algoritmo << ','
               << i << ','
               << j << ','
               << repeticion << ','
               << semilla << ','
               << (registro.medirTiempo
                       ? "tiempo_ns"
                       : "operaciones")
               << ','
               << punto << ','
               << hasta << ','
               << acumulado
               << '\n';
    }

    return acumulado;
}

ResultadoExperimento ejecutarAlgoritmo(
    const Graph& G,
    int raiz,
    bool usarFibonacci,
    bool medirDecreaseKey,
    std::ofstream* curvas,
    int i,
    int j,
    int repeticion,
    std::uint64_t semilla) {

    ResultadoExperimento resultado;

    RegistroDecreaseKey registro;

    if (medirDecreaseKey) {
        registro.medirTiempo = true;
        registro.valores.reserve(G.numVertices() + G.numVertices());
    }

    const auto inicio = Reloj::now();

    if (usarFibonacci) {
        resultado.prim =
            primFibonacci(
                G,
                raiz,
                medirDecreaseKey ? &registro : nullptr
            );
    } else {
        resultado.prim =
            primBinomial(
                G,
                raiz,
                medirDecreaseKey ? &registro : nullptr
            );
    }

    const auto fin = Reloj::now();

    resultado.tiempoTotalMs =
        std::chrono::duration<double, std::milli>(
            fin - inicio
        ).count();

    verificarResultado(
        resultado.prim,
        G.numVertices()
    );

    if (medirDecreaseKey) {
        if (registro.valores.size() !=
            resultado.prim.llamadasDecreaseKey) {
            throw std::runtime_error(
                "Registro de tiempos incompleto");
        }

        if (curvas != nullptr) {
            guardarCurva(
                *curvas,
                registro,
                usarFibonacci ? "fibonacci" : "binomial",
                i,
                j,
                repeticion,
                semilla
            );
        }

        resultado.tiempoDecreaseKeyMs =
            resultado.prim.tiempoDecreaseKey * 1000.0;

        resultado.memoriaRegistro =
            registro.valores.size() *
            sizeof(std::uint64_t);

        /*
         * Segunda pasada para contar operaciones estructurales.
         * No se mide el tiempo de esta pasada.
         */
        registro.medirTiempo = false;

        const PrimResult conteo =
            usarFibonacci
                ? primFibonacci(G, raiz, &registro)
                : primBinomial(G, raiz, &registro);

        if (registro.valores.size() !=
            resultado.prim.llamadasDecreaseKey) {
            throw std::runtime_error(
                "Cantidad de llamadas distinta entre pasadas");
        }

        if (conteo.aristas != resultado.prim.aristas ||
            conteo.pesoTotal != resultado.prim.pesoTotal ||
            conteo.llamadasDecreaseKey !=
                resultado.prim.llamadasDecreaseKey) {
            throw std::runtime_error(
                "Las pasadas de decreaseKey no coinciden");
        }

        const std::uint64_t operaciones =
            guardarCurva(
                *curvas,
                registro,
                usarFibonacci ? "fibonacci" : "binomial",
                i,
                j,
                repeticion,
                semilla
            );

        if (usarFibonacci) {
            if (operaciones != conteo.cortes) {
                throw std::runtime_error(
                    "El registro de cortes no coincide");
            }
        } else {
            if (operaciones != conteo.intercambios) {
                throw std::runtime_error(
                    "El registro de intercambios no coincide");
            }
        }
    }

    return resultado;
}

void guardarEntorno(
    std::ofstream& salida,
    char serie,
    bool piloto) {

    struct utsname sistema{};

    if (uname(&sistema) == 0) {
        salida
            << "Sistema: "
            << sistema.sysname
            << ' '
            << sistema.release
            << "; arquitectura: "
            << sistema.machine
            << '\n';
    }

    std::ifstream cpu("/proc/cpuinfo");
    std::string linea;

    while (std::getline(cpu, linea)) {
        if (linea.rfind("model name", 0) == 0) {
            salida << linea << '\n';
            break;
        }
    }

    salida
        << "Compilador: "
        << __VERSION__
        << "; __cplusplus="
        << __cplusplus
        << '\n'

        << "Serie: "
        << serie
        << "; modo: "
        << (piloto ? "piloto" : "oficial")
        << '\n'

        << "Repeticiones: 10; raiz: 0; "
           "semilla: 42 + 10000*i + 100*j + repeticion\n"

        << "Pesos: uniformes en (0,1]; "
           "generador: mt19937_64\n"

        << "Reloj: steady_clock; periodo: "
        << Reloj::period::num
        << '/'
        << Reloj::period::den
        << " s\n"

        << "RAM total bytes: "
        << memoria("/proc/meminfo", "MemTotal")
        << '\n'

        << "RAM disponible inicial bytes: "
        << memoria("/proc/meminfo", "MemAvailable")
        << '\n'

        << "Algoritmos: binomial y fibonacci\n"

        << "C/D: registro de tiempo y operaciones "
           "en pasadas separadas\n";
}

void ejecutarSerie(
    char serie,
    const std::filesystem::path& carpeta,
    bool piloto) {

    std::filesystem::create_directories(carpeta);

    std::ofstream filas(
        carpeta / "repeticiones.csv");

    std::ofstream promedios(
        carpeta / "promedios.csv");

    std::ofstream curvas(
        carpeta / "curvas.csv");

    std::ofstream entorno(
        carpeta / "entorno.txt");

    for (auto* archivo :
         {&filas, &promedios, &curvas, &entorno}) {

        archivo->exceptions(
            std::ios::failbit |
            std::ios::badbit
        );

        *archivo << std::setprecision(17);
    }

    guardarEntorno(
        entorno,
        serie,
        piloto
    );

    filas
        << "modo,algoritmo,serie,i,j,V,E,"
           "repeticion,semilla,raiz,huella_grafo,"
           "tiempo_prim_ms,decrease_ms,llamadas,"
           "intercambios,cortes,peso_mst,aristas_mst,"
           "registro_bytes,adyacencia_bytes,"
           "ram_disponible_bytes,pico_proceso_bytes,"
           "swap_muestras_bytes,fallos_mayores_prim\n";

    promedios
        << "modo,algoritmo,serie,i,j,V,E,repeticiones,"
           "tiempo_prim_ms,decrease_ms,llamadas,"
           "intercambios,cortes,peso_mst\n";

    curvas
        << "algoritmo,i,j,repeticion,semilla,"
           "medicion,punto,llamadas,acumulado\n";

    const bool total =
        serie == 'A' ||
        serie == 'B';

    const char* modo =
        piloto ? "piloto" : "oficial";

    std::uint64_t picoSerie = 0;
    std::uint64_t swapSerie = 0;
    long fallosSerie = 0;

    std::cout
        << "\nSerie "
        << serie
        << " - "
        << modo
        << " - 10 repeticiones\n";

    for (const auto& [i, j] :
         configuraciones(serie, piloto)) {

        const std::size_t V =
            std::size_t{1} << i;

        const std::size_t E =
            std::size_t{1} << j;

        std::cout
            << "\ni="
            << i
            << ", j="
            << j
            << ", V="
            << V
            << ", E="
            << E
            << "\n";

        struct Acumulados {
            double tiempoTotal = 0.0;
            double tiempoDecreaseKey = 0.0;
            double llamadas = 0.0;
            double intercambios = 0.0;
            double cortes = 0.0;
            double peso = 0.0;
        };

        Acumulados binomial;
        Acumulados fibonacci;

        for (int repeticion = 1;
             repeticion <= repeticiones;
             ++repeticion) {

            const std::uint64_t semilla =
                semillaBase +
                10000 * i +
                100 * j +
                repeticion;

            const auto ramDisponible =
                memoria(
                    "/proc/meminfo",
                    "MemAvailable"
                );

            auto swap =
                memoria(
                    "/proc/self/status",
                    "VmSwap"
                );

            const Graph G =
                generateConnectedGraph(
                    V,
                    E,
                    semilla
                );

            const auto datos =
                identificar(G, E);

            swap = std::max(
                swap,
                memoria(
                    "/proc/self/status",
                    "VmSwap"
                )
            );

            const auto antes =
                fallosMayores();

            ResultadoExperimento RBinomial =
                ejecutarAlgoritmo(
                    G,
                    0,
                    false,
                    !total,
                    &curvas,
                    i,
                    j,
                    repeticion,
                    semilla
                );

            const auto despuesBinomial =
                fallosMayores();

            ResultadoExperimento RFibonacci =
                ejecutarAlgoritmo(
                    G,
                    0,
                    true,
                    !total,
                    &curvas,
                    i,
                    j,
                    repeticion,
                    semilla
                );

            const auto despuesFibonacci =
                fallosMayores();

            const long fallos =
                (despuesBinomial - antes) +
                (despuesFibonacci - despuesBinomial);

            if (std::abs(
                    RBinomial.prim.pesoTotal -
                    RFibonacci.prim.pesoTotal) > 1e-9) {

                throw std::runtime_error(
                    "Binomial y Fibonacci produjeron "
                    "MST con pesos distintos"
                );
            }

            swap = std::max(
                swap,
                memoria(
                    "/proc/self/status",
                    "VmSwap"
                )
            );

            const auto pico =
                memoria(
                    "/proc/self/status",
                    "VmHWM"
                );

            picoSerie =
                std::max(picoSerie, pico);

            swapSerie =
                std::max(swapSerie, swap);

            fallosSerie += fallos;

            filas
                << modo
                << ",binomial,"
                << serie << ','
                << i << ','
                << j << ','
                << V << ','
                << E << ','
                << repeticion << ','
                << semilla << ",0,"
                << datos.huella << ','
                << RBinomial.tiempoTotalMs << ','
                << (total ? 0.0
                          : RBinomial.tiempoDecreaseKeyMs)
                << ','
                << RBinomial.prim.llamadasDecreaseKey
                << ','
                << RBinomial.prim.intercambios
                << ','
                << RBinomial.prim.cortes
                << ','
                << RBinomial.prim.pesoTotal
                << ','
                << RBinomial.prim.aristas.size()
                << ','
                << RBinomial.memoriaRegistro
                << ','
                << datos.capacidadBytes
                << ','
                << ramDisponible
                << ','
                << pico
                << ','
                << swap
                << ','
                << fallos
                << '\n';

            filas
                << modo
                << ",fibonacci,"
                << serie << ','
                << i << ','
                << j << ','
                << V << ','
                << E << ','
                << repeticion << ','
                << semilla << ",0,"
                << datos.huella << ','
                << RFibonacci.tiempoTotalMs << ','
                << (total ? 0.0
                          : RFibonacci.tiempoDecreaseKeyMs)
                << ','
                << RFibonacci.prim.llamadasDecreaseKey
                << ','
                << RFibonacci.prim.intercambios
                << ','
                << RFibonacci.prim.cortes
                << ','
                << RFibonacci.prim.pesoTotal
                << ','
                << RFibonacci.prim.aristas.size()
                << ','
                << RFibonacci.memoriaRegistro
                << ','
                << datos.capacidadBytes
                << ','
                << ramDisponible
                << ','
                << pico
                << ','
                << swap
                << ','
                << fallos
                << '\n';

            filas.flush();
            curvas.flush();

            binomial.tiempoTotal +=
                RBinomial.tiempoTotalMs;

            binomial.tiempoDecreaseKey +=
                RBinomial.tiempoDecreaseKeyMs;

            binomial.llamadas +=
                RBinomial.prim.llamadasDecreaseKey;

            binomial.intercambios +=
                RBinomial.prim.intercambios;

            binomial.cortes +=
                RBinomial.prim.cortes;

            binomial.peso +=
                RBinomial.prim.pesoTotal;

            fibonacci.tiempoTotal +=
                RFibonacci.tiempoTotalMs;

            fibonacci.tiempoDecreaseKey +=
                RFibonacci.tiempoDecreaseKeyMs;

            fibonacci.llamadas +=
                RFibonacci.prim.llamadasDecreaseKey;

            fibonacci.intercambios +=
                RFibonacci.prim.intercambios;

            fibonacci.cortes +=
                RFibonacci.prim.cortes;

            fibonacci.peso +=
                RFibonacci.prim.pesoTotal;

            std::cout
                << "  Rep "
                << repeticion
                << ": MST="
                << std::setprecision(10)
                << RBinomial.prim.pesoTotal
                << ", Binomial="
                << RBinomial.tiempoTotalMs
                << " ms, Fibonacci="
                << RFibonacci.tiempoTotalMs
                << " ms\n";
        }

        const double divisor =
            static_cast<double>(repeticiones);

        promedios
            << modo
            << ",binomial,"
            << serie << ','
            << i << ','
            << j << ','
            << V << ','
            << E << ','
            << repeticiones << ','
            << binomial.tiempoTotal / divisor
            << ','
            << binomial.tiempoDecreaseKey / divisor
            << ','
            << binomial.llamadas / divisor
            << ','
            << binomial.intercambios / divisor
            << ','
            << binomial.cortes / divisor
            << ','
            << binomial.peso / divisor
            << '\n';

        promedios
            << modo
            << ",fibonacci,"
            << serie << ','
            << i << ','
            << j << ','
            << V << ','
            << E << ','
            << repeticiones << ','
            << fibonacci.tiempoTotal / divisor
            << ','
            << fibonacci.tiempoDecreaseKey / divisor
            << ','
            << fibonacci.llamadas / divisor
            << ','
            << fibonacci.intercambios / divisor
            << ','
            << fibonacci.cortes / divisor
            << ','
            << fibonacci.peso / divisor
            << '\n';

        promedios.flush();

        std::cout
            << "  Promedio Binomial: "
            << binomial.tiempoTotal / divisor
            << " ms\n";

        std::cout
            << "  Promedio Fibonacci: "
            << fibonacci.tiempoTotal / divisor
            << " ms\n";

        std::cout
            << "  MST promedio: "
            << binomial.peso / divisor
            << '\n';
    }

    std::cout
        << "\nPico RSS de la serie: "
        << picoSerie / MiB
        << " MiB\n";

    std::cout
        << "Swap maximo observado: "
        << swapSerie / MiB
        << " MiB\n";

    std::cout
        << "Fallos mayores durante Prim: "
        << fallosSerie
        << '\n';

    std::cout
        << "Archivos: "
        << carpeta.string()
        << "\nEstado: 50 ejecuciones por algoritmo completas\n";
}

} // namespace

int ejecutarExperimentos(
    const std::string& serie,
    const std::string& directorio,
    bool piloto) {

    if (serie != "todas" &&
        serie != "A" &&
        serie != "B" &&
        serie != "C" &&
        serie != "D") {

        throw std::invalid_argument(
            "La serie debe ser A, B, C, D o todas"
        );
    }

    const std::string series =
        serie == "todas"
            ? "ABCD"
            : serie;

    for (char s : series) {
        const auto carpeta =
            std::filesystem::path(directorio) /
            std::string(1, s);

        for (const char* nombre :
             {
                 "repeticiones.csv",
                 "promedios.csv",
                 "curvas.csv",
                 "entorno.txt"
             }) {

            if (std::filesystem::exists(
                    carpeta / nombre)) {

                throw std::runtime_error(
                    "Ya existe " +
                    (carpeta / nombre).string() +
                    "; elija otro directorio de salida"
                );
            }
        }
    }

    for (char s : series) {
        ejecutarSerie(
            s,
            std::filesystem::path(directorio) /
                std::string(1, s),
            piloto
        );
    }

    return 0;
}