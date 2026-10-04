# Tarea 1 — Diseño y Análisis de Algoritmos

Implementación del algoritmo de Prim utilizando estructuras de datos **Binomial Heap** y **Fibonacci Heap**, junto con la generación de grafos y el conjunto de experimentos requeridos para comparar el comportamiento de ambas implementaciones.

## Estructura del proyecto

```text
tarea1-logs/
├── src/
│   ├── binomial_heap.cpp
│   ├── binomial_heap.h
│   ├── fibonacci_heap.cpp
│   ├── fibonacci_heap.h
│   ├── graph.cpp
│   ├── graph.h
│   ├── prim.cpp
│   ├── prim.h
│   ├── experiments.cpp
│   ├── experiments.h
│   └── main.cpp
├── .gitignore
└── README.md
```

La carpeta `resultados/`, utilizada durante la ejecución de los experimentos, no forma parte del repositorio. Los resultados finales y gráficos se encuentran incluidos en el informe de la tarea.

## Requisitos

Para compilar y ejecutar el proyecto se requiere:

- Compilador compatible con **C++17**.
- **g++**.
- Sistema operativo Linux o compatible con las herramientas utilizadas.
- Biblioteca estándar de C++.

La implementación utiliza `std::chrono::steady_clock` para realizar las mediciones de tiempo.

## Compilación

Desde la raíz del repositorio ejecutar:

```bash
g++ -std=c++17 -Wall -Wextra -pedantic src/*.cpp -o tarea1
```

Esto generará el ejecutable:

```text
tarea1
```

## Ejecución

Para mostrar las opciones disponibles:

```bash
./tarea1
```

El programa permite realizar una ejecución piloto y ejecutar las cuatro series de experimentos solicitadas.

### Ejecución piloto

La ejecución piloto permite comprobar el funcionamiento de las cuatro series antes de realizar la ejecución oficial:

```bash
./tarea1 --piloto
```

Por defecto, los resultados se generan en:

```text
resultados/piloto/
```

También es posible especificar un directorio de salida:

```bash
./tarea1 --piloto mi_directorio
```

### Ejecución de una serie específica

Para ejecutar una serie determinada:

```bash
./tarea1 --experimentos A
```

```bash
./tarea1 --experimentos B
```

```bash
./tarea1 --experimentos C
```

```bash
./tarea1 --experimentos D
```

Por defecto, los resultados se generan en:

```text
resultados/final/
```

También se puede especificar un directorio de salida:

```bash
./tarea1 --experimentos A resultados/final
```

### Ejecución de todas las series

Para ejecutar las cuatro series:

```bash
./tarea1 --experimentos todas
```

También se puede utilizar:

```bash
./tarea1 --todo
```

Ambas opciones ejecutan las series A, B, C y D.

## Series de experimentos

El programa implementa las cuatro series de experimentos definidas en la tarea.

### Serie A

Mantiene fijo el número de vértices y aumenta progresivamente el número de aristas del grafo.

Esta serie permite estudiar el comportamiento de las implementaciones al aumentar la cantidad de aristas.

### Serie B

Mantiene fijo el número de aristas y aumenta progresivamente el número de vértices.

Esta serie permite estudiar el comportamiento de las implementaciones al aumentar el tamaño del conjunto de vértices.

### Serie C

Mide el comportamiento de las operaciones `decreaseKey` al aumentar el número de aristas.

Se registran:

- Tiempo acumulado de `decreaseKey`.
- Número de llamadas a `decreaseKey`.
- Operaciones estructurales realizadas.

Para el **Binomial Heap** se registran los intercambios realizados.

Para el **Fibonacci Heap** se registran los cortes realizados, incluyendo los cortes producidos mediante `cascadingCut`.

### Serie D

Mide el comportamiento de las operaciones `decreaseKey` al aumentar el número de vértices, manteniendo fijo el número de aristas.

Al igual que en la Serie C, se registra el tiempo acumulado de `decreaseKey` y las operaciones estructurales de ambas estructuras.

## Implementaciones

### Graph

Los archivos:

```text
src/graph.h
src/graph.cpp
```

implementan la representación del grafo mediante listas de adyacencia.

El generador permite construir grafos conexos simples con pesos positivos. Para los experimentos se utiliza una semilla para permitir la generación reproducible de los grafos.

### Binomial Heap

Los archivos:

```text
src/binomial_heap.h
src/binomial_heap.cpp
```

implementan la estructura **Binomial Heap** utilizada por Prim.

Las principales operaciones implementadas son:

- `insert`
- `extractMin`
- `decreaseKey`

La estructura almacena pares correspondientes al costo y vértice, permitiendo realizar las operaciones necesarias para el algoritmo de Prim.

### Fibonacci Heap

Los archivos:

```text
src/fibonacci_heap.h
src/fibonacci_heap.cpp
```

implementan la estructura **Fibonacci Heap**.

Las principales operaciones implementadas son:

- `insert`
- `extractMin`
- `decreaseKey`
- `cut`
- `cascadingCut`
- Consolidación de árboles.

La implementación utiliza nodos marcados para realizar los cortes en cascada asociados a `decreaseKey`.

### Prim

Los archivos:

```text
src/prim.h
src/prim.cpp
```

contienen las dos implementaciones del algoritmo de Prim:

```text
primBinomial(...)
primFibonacci(...)
```

Ambas implementaciones utilizan la misma representación del grafo, permitiendo comparar directamente el comportamiento de las dos estructuras de datos.

Cada ejecución verifica que el resultado corresponda a un árbol de expansión mínima válido.

## Experimentos

Los archivos:

```text
src/experiments.h
src/experiments.cpp
```

contienen la infraestructura utilizada para ejecutar las series experimentales y realizar las mediciones.

Para cada configuración se realizan múltiples repeticiones y se utilizan los mismos grafos para comparar las implementaciones de Binomial Heap y Fibonacci Heap.

Durante las ejecuciones se registran:

- Tiempo total de ejecución.
- Tiempo acumulado de `decreaseKey`.
- Cantidad de llamadas a `decreaseKey`.
- Intercambios realizados por el Binomial Heap.
- Cortes realizados por el Fibonacci Heap.
- Peso total del árbol de expansión mínima.

También se verifica que ambas implementaciones obtengan el mismo peso total del árbol de expansión mínima para cada grafo.

## Archivos de salida

Las ejecuciones experimentales generan archivos como:

```text
repeticiones.csv
promedios.csv
curvas.csv
entorno.txt
```

### `repeticiones.csv`

Contiene los resultados correspondientes a cada repetición de cada configuración experimental.

### `promedios.csv`

Contiene los valores promedio obtenidos para cada configuración experimental.

### `curvas.csv`

Contiene los datos utilizados para construir las curvas de tiempo acumulado y operaciones estructurales de las Series C y D.

### `entorno.txt`

Contiene información sobre el entorno utilizado para realizar los experimentos, incluyendo información del sistema, procesador, compilador y parámetros de ejecución.

Los archivos de resultados no se incluyen en el repositorio y la carpeta `resultados/` se encuentra excluida mediante `.gitignore`.

## Medición de tiempos

Las mediciones de tiempo se realizan utilizando:

```cpp
std::chrono::steady_clock
```

La medición del tiempo total excluye la generación del grafo y la preparación de los datos, permitiendo comparar el tiempo asociado a la ejecución de los algoritmos.

Para las Series C y D también se mide de forma independiente el tiempo acumulado de las operaciones `decreaseKey`.

## Verificación de resultados

Durante los experimentos se realizan diferentes verificaciones para asegurar la consistencia de los resultados:

- Ambas implementaciones trabajan sobre el mismo grafo en cada repetición.
- El árbol de expansión mínima contiene `V - 1` aristas.
- Ambas implementaciones obtienen el mismo peso total del árbol de expansión mínima.
- Se verifica que se hayan realizado las llamadas necesarias a `decreaseKey`.
- Las mediciones de las operaciones estructurales son consistentes con las operaciones realizadas por cada estructura.

## Complejidad

Las implementaciones están diseñadas de acuerdo con las complejidades teóricas esperadas para el algoritmo de Prim utilizando cada estructura:

- **Prim con Binomial Heap:** `O(E log V)`
- **Prim con Fibonacci Heap:** `O(E + V log V)`

En el caso del Fibonacci Heap, la operación `decreaseKey` posee costo amortizado `O(1)`, utilizando las operaciones `cut` y `cascadingCut`.

## Autores

- **Benjamín Durán**
- **Camila Paredes**