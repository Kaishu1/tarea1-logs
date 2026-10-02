# tarea1-logs

## Compilación

Se utiliza C++17. Desde la raíz del repositorio:

```bash
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic src/*.cpp -o /tmp/tarea1
```

## Pruebas y consumo de memoria

```bash
/tmp/tarea1 --pruebas
/tmp/tarea1 --memoria
/tmp/tarea1 --ayuda
```

Sin argumentos se ejecutan las pruebas pequeñas. `--memoria` ejecuta el caso
de §6.2: V = 2^15, E = 2^20, semilla 42 y raíz 0. Requiere Linux con `/proc`.
Imprime tamaños de tipos, fórmulas, estimación por componente, comparación con
la RAM disponible, consumo observado y estado de ejecución. Incluye
proyecciones de casos mayores, pero no ejecuta las series A–D.

Para guardar la salida mientras se muestra en terminal:

```bash
/tmp/tarea1 --memoria | tee memoria-binomial.txt
```

El desglose y las limitaciones de la estimación están en
[docs/memoria_binomial.md](docs/memoria_binomial.md). La RAM disponible y las
mediciones del proceso se consultan de nuevo en cada ejecución. Para medir
memoria, usar la compilación indicada, sin sanitizadores.

## Experimentos binomiales de la sección 6

Ejecutar memoria y las cuatro series oficiales:

```bash
/tmp/tarea1 --todo resultados/binomial
```

También se puede ejecutar cada serie por separado, o las cuatro sin repetir
la comprobación de memoria:

```bash
/tmp/tarea1 --experimentos A resultados/binomial
/tmp/tarea1 --experimentos B resultados/binomial
/tmp/tarea1 --experimentos C resultados/binomial
/tmp/tarea1 --experimentos D resultados/binomial
# Alternativa a los cuatro comandos anteriores:
/tmp/tarea1 --experimentos todas resultados/otra-ejecucion
```

| Serie | Configuraciones oficiales | Medición |
|---|---|---|
| A | i=20, j=20,21,22,23,24 | Tiempo total de Prim |
| B | j=24, i=18,19,20,21,22 | Tiempo total de Prim |
| C | i=18, j=18,19,20,21,22 | Tiempo de decreaseKey e intercambios |
| D | j=22, i=14,15,16,17,18 | Tiempo de decreaseKey e intercambios |

Siempre V=2^i, E=2^j, raíz 0 y **10 grafos por configuración**. Cada grafo
usa `semilla = 42 + 10000*i + 100*j + repeticion`, con repetición de 1 a 10.
Las configuraciones compartidas por dos series reutilizan las mismas semillas.
Se genera y procesa un solo grafo a la vez. No se reducen tamaños automáticamente.
El párrafo de gráficos de §6.3.2 menciona A/B; aquí se usan C/D, las series
definidas en esa misma sección para el costo amortizado.

Antes de una ejecución grande se puede comprobar el flujo con tamaños pequeños:

```bash
/tmp/tarea1 --piloto resultados/piloto
```

El piloto también hace diez repeticiones y produce los mismos archivos. Sus
resultados se identifican como `piloto` y no sustituyen los tamaños oficiales.
Sin directorio explícito se usan `resultados/binomial` y `resultados/piloto`.
Los resultados existentes no se sobrescriben: para repetir una serie, elegir
otro directorio. Una ejecución incompleta conserva sus filas terminadas y no
genera promedios de configuraciones incompletas; para repetirla, usar otra carpeta.

### Mediciones y salida

La terminal muestra V, E, semilla, repetición, tiempo en ms, llamadas,
intercambios, peso del MST y promedios por configuración. También indica las
muestras de swap o los fallos de página mayores cuando aparecen, y el pico
de memoria alcanzado hasta el final de cada serie. Cada carpeta de serie
contiene:

- `repeticiones.csv`: resultados individuales, identificación del grafo,
  tamaño del MST, memoria reservada para registros, capacidad de adyacencia,
  RAM disponible, pico RSS del proceso, muestras de swap y fallos de página mayores.
- `promedios.csv`: promedios aritméticos de las diez repeticiones, incluido
  el promedio de intercambios por llamada (también cuentan llamadas con cero).
- `curvas.csv`: prefijos acumulados de tiempo y operaciones para C/D.
  En A/B solo contiene el encabezado.
- `entorno.txt`: compilador, sistema, CPU, RAM y parámetros de medición.

Los campos de tiempo que no corresponden a una serie quedan vacíos en el CSV.
El peso medio del MST resume diez grafos distintos; la comparación entre colas
debe usar los pesos **individuales** de `repeticiones.csv`.

En A/B se usa `std::chrono::steady_clock` alrededor de `primBinomial`: incluye
la construcción de la cola y la liberación de sus estructuras locales. No
incluye generación, identificación del grafo, impresión ni escritura de CSV.
No se consulta el reloj por cada decreaseKey en estas series.

En C/D se hacen dos pasadas sobre el mismo grafo. La primera cronometra cada
decreaseKey; la segunda cuenta sus intercambios sin consultar el reloj. Se
comprueba que ambas produzcan exactamente el mismo MST y los mismos contadores.
Cada pasada guarda un entero de 64 bits por llamada, reservando E posiciones
antes de Prim (8E bytes en este entorno). Se reutiliza esa reserva y se conserva
el primer MST para la comparación. Los registros y las sumas se procesan después
de Prim; su escritura no está dentro del intervalo cronometrado. Los tiempos
por llamada sí incluyen el costo de consultar el reloj y pueden tener ruido.

Para las curvas se guardan 101 prefijos, correspondientes al 0%, 1%, ..., 100%
del número de llamadas de cada repetición. **Todas** las llamadas contribuyen
a las sumas, aunque solo se exporten esos puntos. Los gráficos promedian los
prefijos del mismo porcentaje de avance de las diez repeticiones: tanto la
coordenada de llamadas como el costo acumulado son promedios. Esto evita
equiparar secuencias que pueden tener distintas longitudes.

El pico RSS es el máximo alcanzado por el proceso hasta cada fila, incluida
la generación; no es un pico aislado de cada Prim. Las muestras de swap no
descartan actividad entre muestras. Las estimaciones excluyen el conjunto
temporal del generador y la administración del asignador. Registrar cualquier
ajuste externo a la máquina; el programa no modifica su configuración.

Para conservar también la terminal y completar los datos del entorno:

```bash
set -o pipefail
/tmp/tarea1 --todo resultados/corrida-2 | tee corrida-2-terminal.txt
lscpu > corrida-2-cpu.txt
```

Usar la compilación indicada al principio y registrar sus flags. Evitar otras
cargas intensivas durante las mediciones. No incluir los grafos generados en
la entrega; el programa conserva mediciones y semillas.

## Gráficos

Se necesita Python 3 y Matplotlib. Instalarlo en un entorno virtual (en Ubuntu,
`python3-venv` debe estar disponible):

```bash
python3 -m venv /tmp/tarea1-graficos
/tmp/tarea1-graficos/bin/python -m pip install matplotlib
/tmp/tarea1-graficos/bin/python scripts/graficar_binomial.py resultados/binomial
```

Genera seis gráficos en PNG dentro de `resultados/binomial/graficos`:
`A_total`, `B_total`, `C_tiempo`, `C_intercambios`, `D_tiempo`, `D_intercambios`.
También funciona con una sola serie completada o con el piloto. Rechaza series
incompletas. Las curvas C/D incluyen las cinco configuraciones de cada serie.

Las referencias son `c E log2(V)` para el tiempo total y `c k log2(V)` para los
acumulados, donde k es el número de llamadas. Para cada gráfico se elige
`c = max(costo_medido / modelo)` sobre sus puntos positivos. Es un ajuste a
las observaciones, no una garantía sobre futuras ejecuciones. Las constantes
quedan en `constantes.csv`; las unidades son ms en gráficos de tiempo e
intercambios en los de operaciones.

Los límites de los ejes se guardan en `escalas.json`. Para comparar con los
gráficos de Fibonacci, acordar límites que incluyan ambas implementaciones y
usar los mismos en sus gráficos. El script acepta esos límites:

```bash
/tmp/tarea1-graficos/bin/python scripts/graficar_binomial.py resultados/binomial \
    --escalas escalas-comunes.json
```

El JSON tiene claves como `A_total` y `C_tiempo`, cada una con `x: [min, max]`
e `y: [min, max]`, como en el archivo generado. El script rechaza límites que
oculten puntos de la curva medida o de su referencia teórica.

## Comparación con Fibonacci

La implementación Fibonacci sigue a cargo del compañero. Para comparar,
ejecutar ambas versiones sobre los mismos grafos: mismos tamaños, semilla,
generador, entorno de biblioteca C++ y raíz. El identificador `huella_grafo`
se calcula con FNV-1a de 64 bits sobre tamaños, orden de vecinos y bits de
los pesos; su definición está en `identificar` de `src/experiments.cpp`.

Cuando estén disponibles sus resultados:

```bash
python3 scripts/comparar_mst.py resultados/binomial resultados/fibonacci
```

La carpeta Fibonacci debe contener `A/repeticiones.csv`, etc., con los campos
`modo,serie,i,j,repeticion,semilla,raiz,huella_grafo,peso_mst,aristas_mst`.
No requiere columnas de tiempo ni de intercambios para esta comparación.
Se exigen las mismas filas y huellas; se comprueba V-1 aristas y pesos finitos
con tolerancia absoluta y relativa de 1e-9. Imprime cada comparación y devuelve
un código distinto de cero ante diferencias. Hasta disponer de esos datos,
la comparación entre ambas implementaciones queda pendiente.
