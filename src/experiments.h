#ifndef EXPERIMENTS_H
#define EXPERIMENTS_H

#include <string>

// Ejecuta A, B, C, D o todas, con diez semillas por configuracion y raiz cero.
// piloto usa tamanos pequenos. Imprime resultados y crea CSV en directorio/serie.
// Rechaza archivos de resultados existentes para evitar sobrescribir mediciones.
// Devuelve cero al completar; lanza una excepcion si falla alguna medicion.
int ejecutarExperimentos(const std::string& serie, const std::string& directorio,
                         bool piloto = false);

#endif
