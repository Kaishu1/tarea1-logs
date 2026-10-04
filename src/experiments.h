#ifndef EXPERIMENTS_H
#define EXPERIMENTS_H

#include <string>

// Ejecuta la serie indicada (A-D o todas) en directorio; piloto usa tamaños reducidos.
// Escribe CSV y entorno por serie; devuelve 0 o lanza si la serie no es válida o hay errores.
int ejecutarExperimentos(const std::string& serie, const std::string& directorio,
                         bool piloto = false);

#endif
