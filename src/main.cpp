#include "experiments.h"

#include <iostream>
#include <stdexcept>
#include <string>

// Ejecuta la serie de experimentos indicada. Piloto usa tamaños pequeños. Imprime resultados y crea CSV en directorio/serie. Rechaza archivos de resultados existentes para evitar sobrescribir mediciones. Devuelve cero al completar; lanza una excepción si falla alguna medición.
int main(int argc, char* argv[]) {
    try {
        if (argc == 1) {
            std::cout
                << "Uso:\n"
                << "  " << argv[0] << " --piloto [directorio]\n"
                << "  " << argv[0]
                << " --experimentos A|B|C|D|todas [directorio]\n"
                << "  " << argv[0] << " --todo [directorio]\n";
            return 0;
        }

        const std::string opcion = argv[1];

        if (opcion == "--piloto") {
            const std::string directorio =
                argc >= 3
                    ? argv[2]
                    : "resultados/piloto";

            return ejecutarExperimentos(
                "todas",
                directorio,
                true
            );
        }

        if (opcion == "--experimentos") {
            if (argc < 3) {
                throw std::invalid_argument(
                    "Debe indicar A, B, C, D o todas"
                );
            }

            const std::string serie = argv[2];

            const std::string directorio =
                argc >= 4
                    ? argv[3]
                    : "resultados/final";

            return ejecutarExperimentos(
                serie,
                directorio,
                false
            );
        }

        if (opcion == "--todo") {
            const std::string directorio =
                argc >= 3
                    ? argv[2]
                    : "resultados/final";

            return ejecutarExperimentos(
                "todas",
                directorio,
                false
            );
        }

        if (opcion == "--ayuda" ||
            opcion == "--help") {

            std::cout
                << "Uso:\n"
                << "  " << argv[0] << " --piloto [directorio]\n"
                << "  " << argv[0]
                << " --experimentos A|B|C|D|todas [directorio]\n"
                << "  " << argv[0] << " --todo [directorio]\n";

            return 0;
        }

        throw std::invalid_argument(
            "Opcion no reconocida: " + opcion
        );

    } catch (const std::bad_alloc&) {
        std::cerr
            << "Error de memoria: no se pudo completar "
               "la ejecucion.\n";
        return 1;

    } catch (const std::exception& error) {
        std::cerr
            << "Error: "
            << error.what()
            << '\n';
        return 1;
    }
}