# L09: Medición de Rendimiento y Consumo de Recursos

## Introducción

Este laboratorio introduce un conjunto de técnicas para medir el tiempo de ejecución y el consumo de recursos de programas en C sin recurrir a herramientas especializadas. La problemática central es: dado un programa que funciona correctamente ¿cuánto tarda y cuánta memoria consume? Y cuando existen más de una implementación ¿cuál es más eficiente y cómo cambia su tiempo de ejecución al aumentar el tamaño del input?

Para responder esas preguntas se usarán cuatro herramientas disponibles en cualquier sistema Linux sin instalación adicional: el comando `time` del shell, el ejecutable `/usr/bin/time -v`, la función `clock_gettime` de la biblioteca POSIX y la llamada al sistema `getrusage`. Cada una tiene un rol distinto: `time` permite medir desde fuera el tiempo total de una ejecución, `/usr/bin/time -v` agrega información sobre memoria y cambios de contexto, mientras que `clock_gettime` y `getrusage` permiten instrumentar el programa (de una forma similar a obtener el estado de variables con `printf`) para medir secciones específicas de código.

El laboratorio se apoya en dos implementaciones distintas de un sistema para planificación de rutas, con una versión que usa bubble sort y una que usa `qsort`. Ambas implementaciones producen resultados idénticos para cualquier entrada, pero sus tiempos de ejecución divergen drásticamente a medida que crece el número de segmentos a procesar.

### Pre-requisitos

- Tener iniciada la máquina virtual Lubuntu y clonar este repositorio
- Tener disponibles `gcc`, `diff`, `awk`, `bash` y `chmod`
- Haber trabajado previamente con terminal, compilación en C y pruebas automatizadas
- Haber completado los laboratorios anteriores de debugging por instrumentación, con `gdb` y con `valgrind`

### Objetivo general

- Medir y comparar el tiempo de ejecución y el consumo de recursos de dos implementaciones del mismo algoritmo usando herramientas estándar del sistema, sin depender de profilers especializados

### Objetivos específicos

- Distinguir entre tiempo de ejecución real, tiempo de CPU de usuario y tiempo de CPU de sistema
- Usar el comando `time` del shell para medir el tiempo total de ejecución de un programa
- Interpretar el reporte completo de `/usr/bin/time -v` incluyendo memoria RSS, fallos de página y cambios de contexto
- Implementar en C la función auxiliar `elapsed_seconds` usando `struct timespec`
- Instrumentar un programa en C con `clock_gettime(CLOCK_MONOTONIC)` para medir una sección específica de código
- Consultar el consumo de memoria desde dentro de un programa C usando `getrusage`
- Verificar que la instrumentación interna no altera la salida funcional del programa
- Comparar los tiempos de ejecución de ambas implementaciones para distintos tamaños de entrada
- Observar empíricamente cómo cambia el tiempo de ejecución al aumentar el tamaño de la entrada

### Metodología

El laboratorio está dividido en dos bloques, uno por programa. El primer bloque gira completamente en torno a `measure_demo`: primero se observa qué información entrega el comando `time` aplicado externamente sobre el esqueleto ya compilable, y luego se implementa la instrumentación interna completando los `TODO`. Esto permite aprender las APIs de medición de forma aislada, sin mezclarlas con la lógica del problema principal. El segundo bloque aplica ese mismo patrón aprendido a los programas `navigation_v1` y `navigation_v2`, verificando que la instrumentación no altera la salida funcional y terminando con un benchmark comparativo.

El foco del laboratorio está en que la medición de tiempo y memoria se implementa de forma muy similar a cómo en laboratorios anteriores se agregaban prints de debug: es código de instrumentación que observa sin modificar la lógica del programa.

### Estructura del repositorio

Este laboratorio usa tres carpetas en paralelo: `code`, `tests` y `scripts`:

- La carpeta `code` contiene el código de los programas a analizar
- La carpeta `tests` contiene casos `testNNN.in` y `testNNN.expected` de la misma forma que se han utilizado en los laboratorios anteriores
- La carpeta `scripts` contiene `tests-run.sh` y `tests-clean.sh` con el mismo flujo de los laboratorios anteriores, más `benchmark.sh` para la comparación de rendimiento

### Contexto

El sistema de planificación de transporte del Imperio Galáctico debe calcular rutas de evacuación a través de múltiples saltos al hiperespacio, cada uno con un costo de combustible asociado. Para seleccionar los caminos más económicos, un módulo de planificación recibe una lista de saltos y los ordena de menor a mayor costo antes de calcular estadísticas sobre la ruta resultante.

El módulo fue implementado originalmente bajo la presión antes de la Batalla de Yavin, resultando en la versión `v1` que funciona correctamente para flotas pequeñas. Con la expansión del Imperio y la necesidad de planificar evacuaciones de sistemas enteros, el volumen de saltos creció de decenas a decenas de miles. Un nuevo equipo de ingeniería propuso la versión `v2`. La flota necesita evidencia objetiva: ¿cuál implementación es más apropiada para misiones de gran envergadura?

## Actividad

### 1. Compilar measure_demo

Entrar a la carpeta `code` y compilar el esqueleto de `measure_demo`:

```bash
cd code
gcc -Wall -Wextra -std=c11 -o measure_demo measure_demo.c
```

### 2. Medición externa con time sobre measure_demo

Antes de escribir cualquier código, observar qué información entrega el shell sobre un programa en ejecución. Ejecutar `measure_demo` para N=1000000:

```bash
./measure_demo 1000000
```

Este programa calcula la suma de los N primeros números naturales, sumando uno a uno. El resultado puede ser comprobado mediante la fórmula de Gauss.

Una vez probado el programa, podemos utilizar el comando `time` para medir el tiempo de ejecución para la suma de diferentes cantidades de números:

```bash
time ./measure_demo 1000000
time ./measure_demo 10000000
```

La salida de `time` tiene tres campos:

- `real`: tiempo transcurrido desde el inicio hasta el final del proceso, como un cronómetro de pulsera
- `user`: tiempo de CPU que el proceso pasó ejecutando código en espacio de usuario
- `sys`: tiempo de CPU que el proceso pasó en llamadas al sistema operativo

Para un proceso que solo calcula en CPU, `user ≈ real`. Para un proceso que duerme o espera I/O (por ejemplo muchos accesos a disco, memoria o internet), `real >> user`. Para ver ese contraste, ejecutar:

```bash
time sleep 2
```

Aquí `real ≈ 2s` porque el proceso estuvo bloqueado, pero `user` y `sys` son casi cero porque durante esos dos segundos no ejecutó código propio ni llamadas al sistema.

Para obtener más información, incluyendo el uso de memoria, usar `/usr/bin/time -v`:

```bash
/usr/bin/time -v ./measure_demo 10000000
```

La salida incluye campos adicionales:

```text
Command being timed: "./measure_demo 10000000"
User time (seconds): 0.02
System time (seconds): 0.01
Percent of CPU this job got: 100%
Elapsed (wall clock) time (h:mm:ss or m:ss): 0:00.04
Average shared text size (kbytes): 0
Average unshared data size (kbytes): 0
Average stack size (kbytes): 0
Average total size (kbytes): 0
Maximum resident set size (kbytes): 79196
Average resident set size (kbytes): 0
Major (requiring I/O) page faults: 0
Minor (reclaiming a frame) page faults: 19608
Voluntary context switches: 0
Involuntary context switches: 2
Swaps: 0
File system inputs: 0
File system outputs: 0
Socket messages sent: 0
Socket messages received: 0
Signals delivered: 0
Page size (bytes): 4096
Exit status: 0
```

Los campos más relevantes son `Maximum resident set size` (máximo de memoria RAM física usada en kilobytes) y los cambios de contexto. Un valor de `Voluntary context switches` cercano a cero confirma que el programa no estuvo esperando I/O durante su ejecución.

> ℹ️ **Información**: Un "cambio de contexto" corresponde cuando el sistema operativo cambia qué programa está utilizando el procesador. Los detalles sobre esta problemática y las consecuencias para los programas se verán más adelante en la carrera en la asignatura de Sistemas Operativos.

Ejecutar con distintos valores de N y observar cómo evoluciona el `Maximum resident set size`. Anotar los valores de `User time` y `Maximum resident set size` para N=1000000 y N=10000000. Se usarán en la sección siguiente para verificar que la instrumentación interna es consistente con lo que reporta el shell.

### 3. Implementar la instrumentación interna de measure_demo

El archivo `measure_demo.c` tiene comentarios `TODO` que marcan exactamente qué falta implementar. Abrir el archivo con `vim` y completar las cinco partes:

#### 3.1 Implementar na función auxiliar `elapsed_seconds`

La función debe recibir dos `struct timespec` y debe retornar la diferencia en segundos. Un `struct timespec` tiene dos campos: `tv_sec` (segundos enteros) y `tv_nsec` (nanosegundos fraccionarios). El código provee la función comentada. Completar el cuerpo de la función con:

```c
return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) * 1e-9;
```

Osea, la diferencia de segundos más la diferencia de nanosegundos multiplicados por 1e-9.

> ℹ️ **Información**: 1e-9 corresponde a una notación de potencias de 10, de la forma AeB que equivale a A * 10^B

#### 3.2 Declarar variables de medición

En `main`, donde dice el primer bloque `TODO`, escribir las tres declaraciones:

```c
struct timespec t_start, t_end;
clock_t cpu_start, cpu_end;
struct rusage usage;
```

Los `structs` son una forma de construir estructuras de datos que agrupan varios valores simultaneamente, de la misma forma que se utilizó en el timespec de la sección 3.1. En este caso, `rusage` corresponde a una struct que se carga desde bibliotecas, al igual que el tipo de dato `clock_t`.

#### 3.3 Capturar el estado inicial

La parte del programa que corresponde a cómputo comienza luego de terminar la validación de los datos, en el momento de comenzar con la reserva de memoria. Colocar justo antes del `malloc`:

```c
clock_gettime(CLOCK_MONOTONIC, &t_start);
cpu_start = clock();
```

`clock_gettime` escribe la hora actual del reloj en `t_start`. `clock()` retorna el tiempo de CPU acumulado del proceso en unidades de `CLOCKS_PER_SEC`. `CLOCK_MONOTONIC` garantiza que el reloj nunca retrocede aunque se cambie la hora del sistema.

#### 3.4 Capturar el estado final e imprimir

Al terminar el proceso, pero antes de terminar el programa, justo después del `free(data)`, agregar:

```c
clock_gettime(CLOCK_MONOTONIC, &t_end);
cpu_end = clock();
getrusage(RUSAGE_SELF, &usage);
```

Para poder obtener el tiempo al terminar y poder calcular el tiempo empleado. 

#### 3.5 Imprimir los resultados

Agregar las siguientes tres líneas para imprimir los resultados antes de terminar el programa:

```c
printf("Tiempo real    : %.6f s\n", elapsed_seconds(t_start, t_end));
printf("Tiempo CPU     : %.6f s\n", (double)(cpu_end - cpu_start) / CLOCKS_PER_SEC);
printf("Memoria RSS    : %ld KB\n", usage.ru_maxrss);
```

#### 3.6 Recompilar y ejecutar

Recompilar y ejecutar con los mismos valores de N usados en la sección 2:

```bash
./measure_demo 1000000
./measure_demo 10000000
```

La salida completa debe tener esta forma:

```text
N              : 10000000
Suma           : 50000005000000.00
Tiempo real    : 0.049170 s
Tiempo CPU     : 0.049096 s
Memoria RSS    : 79196 KB
```

Comparar el `Tiempo real` y el `Memoria RSS` reportados internamente con los valores `User time` y `Maximum resident set size` anotados en la sección 2. Deben ser muy similares: esto confirma que la instrumentación interna y la medición externa del shell son consistentes entre sí. La diferencia residual se debe a que el shell mide el proceso completo (incluyendo carga del ejecutable), mientras que la instrumentación interna solo mide desde la primera llamada a `clock_gettime`.

### 4. Compilar los programas de navegación

Compilar `navigation_v1` y `navigation_v2` desde la carpeta `code`:

```bash
gcc -Wall -Wextra -std=c11 -o navigation_v1 navigation_v1.c
gcc -Wall -Wextra -std=c11 -o navigation_v2 navigation_v2.c
```

Ambos programas leen una lista de costos de saltos en el hiperespacio desde `stdin` y producen estadísticas sobre esa lista. Para poder medir su rendimiento con entradas grandes se necesita generar datos sintéticos. Esto se hace con `awk`:

```bash
awk 'BEGIN { srand(42); print 16384; for(i=1;i<=16384;i++) printf "%.4f\n", rand()*10000 }' \
  > /tmp/random_list.txt
```

`awk` es una herramienta de procesamiento de texto que también funciona como un lenguaje de scripting pequeño. El bloque `BEGIN { ... }` se ejecuta una sola vez antes de procesar cualquier línea de entrada; como aquí no hay archivo de entrada, es donde ocurre todo el trabajo:

- `srand(42)`: inicializa el generador de números pseudo-aleatorios con la semilla 42, igual que `srand` en C. Usar siempre la misma semilla garantiza que los números generados sean idénticos en cada ejecución
- `print 16384`: imprime el número de segmentos en la primera línea, que es lo que el programa espera leer primero
- `for(i=1; i<=16384; i++) printf "%.4f\n", rand()*10000`: imprime 16384 valores flotantes aleatorios entre 0 y 10000, uno por línea. `rand()` en `awk` devuelve un valor entre 0 y 1, por eso debe ser multiplicado

El resultado se redirige con `>` a un archivo temporal para poder reutilizarlo en varias ejecuciones sin regenerarlo cada vez.

La carpeta `/tmp` es una carpeta especial del sistema Linux destinada a archivos temporales. Se usa para guardar datos intermedios que solo se necesitan durante una sesión o durante la ejecución de algunos comandos, sin mezclarlos con los archivos permanentes del sistema. En este laboratorio se utiliza porque conviene generar una vez el input grande y luego reutilizarlo varias veces para comparar programas, medir tiempos y redirigir salidas, pero sin dejar ese archivo como parte del material del laboratorio.

En laboratorios anteriores no había sido necesario usar `/tmp` porque los inputs de prueba ya venían preparados dentro de la carpeta `tests`. Aquí, en cambio, estamos construyendo un caso grande artificial solo para medición de rendimiento, por lo que tiene más sentido dejarlo en una ubicación temporal.

Finalmente, verificamos que ambos programas producen exactamente la misma salida para el mismo input:

```bash
./navigation_v1 < /tmp/random_list.txt
./navigation_v2 < /tmp/random_list.txt
```

Las cinco líneas de salida deben ser idénticas en ambos casos. Este es el punto de partida: dos programas funcionalmente equivalentes cuyo rendimiento aún no se ha medido de forma interna.

### 5. Instrumentar navigation_v1.c

`navigation_v1.c` ya incluye las extensiones POSIX y las bibliotecas necesarias, solo falta instrumentar. La instrumentación envuelve la llamada a `bubble_sort` y los resultados se imprimen por `stderr` para no afectar la salida funcional.

#### 5.1 Instrumentación

El archivo ya trae disponible una instrumentación mínima basada en `clock_gettime` y `getrusage`. Para activarla, descomentar los tres bloques `TODO` que rodean la llamada a `bubble_sort`:

- `TODO 5.1`: declara `t0`, `t1` y `usage` para almacenar las mediciones
- `TODO 5.2`: captura el instante inicial justo antes de comenzar el ordenamiento
- `TODO 5.3`: captura el instante final, consulta el RSS con `getrusage` e imprime por `stderr` el tiempo de ordenamiento y la memoria usada

De esta forma, el programa sigue entregando las estadísticas funcionales por `stdout`, pero además reporta por `stderr` cuánto tardó específicamente el `bubble_sort` y cuál fue el máximo RSS del proceso.

#### 5.2 Recompilar y verificar

```bash
gcc -Wall -Wextra -std=c11 -o navigation_v1 navigation_v1.c
```

Ejecutar con un input pequeño y confirmar que `stdout` produce la misma salida de siempre y las métricas aparecen en `stderr`:

```bash
./navigation_v1 << EOF
4
1.00
3.00
2.00
4.00
EOF
```

#### 5.3 Confirmar que la instrumentación no rompe la suite de pruebas

Correr la suite de pruebas sobre la versión instrumentada:

```bash
chmod +x scripts/tests-run.sh scripts/tests-clean.sh scripts/benchmark.sh
./scripts/tests-run.sh ./code/navigation_v1 ./tests
```

Todos los tests deben seguir en `PASS`. Esto confirma que los `fprintf(stderr, ...)` de la instrumentación no interfieren con la comparación de salida, ya que el script solo captura y compara `stdout`.

Este es el mismo principio que justificaba el uso de `stderr` en el laboratorio de debugging: el canal de diagnóstico no contamina el canal funcional.

### 6. Instrumentar navigation_v2.c

Repetir el proceso de la sección 5 sobre `navigation_v2.c`. Los bloques a descomentar son los mismos, con `qsort` como la llamada instrumentada en lugar de `bubble_sort`.

#### 6.1 Instrumentación

`navigation_v2.c` también incluye una instrumentación lista para activar alrededor de `qsort`. Para habilitarla, descomentar los tres bloques `TODO` del archivo:

- `TODO 6.1`: declara `t0`, `t1` y `usage`
- `TODO 6.2`: captura el instante inicial antes de ejecutar `qsort`
- `TODO 6.3`: captura el instante final, consulta el RSS e imprime por `stderr` el tiempo de ordenamiento y la memoria usada

La idea es exactamente la misma que en `navigation_v1.c`: medir desde dentro del programa la sección crítica de ordenamiento sin alterar la salida funcional.

#### 6.2 Recompilar y verificar

Recompilar en la carpeta code mediante:

```bash
gcc -Wall -Wextra -std=c11 -o navigation_v2 navigation_v2.c
```

Correr la suite de pruebas en la raiz del repositorio mediante:

```bash
./scripts/tests-run.sh ./code/navigation_v2 ./tests
```

También debe pasar todos los tests.

### 7. Benchmark y comparación de tiempos

Con ambos programas instrumentados, ejecutarlos sobre el input grande generado en la sección 4 para observar la instrumentación interna en acción:

```bash
./code/navigation_v1 < /tmp/random_list.txt
./code/navigation_v2 < /tmp/random_list.txt
```

Para ver la salida de `stderr` de forma aislada:

```bash
./code/navigation_v1 < /tmp/random_list.txt 2>/tmp/v1_stats.txt
cat /tmp/v1_stats.txt
```

Confirmar que el tiempo de ordenamiento reportado internamente es consistente con lo que reporta el comando `time` al ejecutar el mismo programa sobre el mismo input. La pequeña diferencia residual se debe a que `time` mide el proceso completo (carga del ejecutable, lectura de datos) mientras que la instrumentación interna solo mide el bloque de ordenamiento.

Para obtener una visión comparativa sistemática sobre distintos tamaños de input, ejecutar el script de benchmark:

```bash
./scripts/benchmark.sh ./code/navigation_v1 ./code/navigation_v2
```

La salida reporta el tiempo real de cada corrida para N=1000 hasta N=32000:

```text
==================================================
 Benchmark: v1 (bubble sort) vs v2 (qsort)
==================================================
N         v1 real (s)     v2 real (s)
--------  --------------  --------------
1000      0.002           0.001
2000      0.007           0.001
4000      0.029           0.001
8000      0.118           0.002
16384     0.470           0.003
32000     1.884           0.007
==================================================
```

Los valores exactos dependerán del hardware, pero la tendencia debería ser clara. Completar una tabla con los tiempos reales obtenidos y describir qué programa aumenta más rápido su tiempo al crecer el input:

| N     | v1 (s) | v2 (s) |
|-------|--------|--------|
| 1000  | —      | —      |
| 2000  | —      | —      |
| 4000  | —      | —      |
| 8000  | —      | —      |
| 16384 | —      | —      |
| 32000 | —      | —      |

La comparación debe permitir responder de forma experimental cuál de las dos implementaciones mantiene tiempos más bajos y cuál empeora cuando aumenta la cantidad de datos.
