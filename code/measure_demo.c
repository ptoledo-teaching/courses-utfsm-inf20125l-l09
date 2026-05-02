#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/resource.h>

/* Retorna la diferencia en segundos entre dos capturas de clock_gettime.
   Restar tv_sec de ambas y sumar tv_nsec convertido a segundos (* 1e-9). */

/*
static double elapsed_seconds(struct timespec start, struct timespec end)
{
    // TODO 3.1: Implementar función
    return 0.0;
}
*/

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Uso: %s <N>\n", argv[0]);
        return 1;
    }

    long n = atol(argv[1]);
    if (n <= 0)
    {
        fprintf(stderr, "Error: N debe ser un entero positivo\n");
        return 1;
    }

    /* TODO 3.2: Declarar las variables para medición */

    /* TODO 3.3: Capturar inicio del programa */

    double *data = malloc((size_t)n * sizeof(double));
    if (data == NULL)
    {
        fprintf(stderr, "Error: no se pudo reservar memoria para %ld elementos\n", n);
        return 1;
    }

    for (long i = 0; i < n; i++)
    {
        data[i] = (double)(i + 1);
    }

    double sum = 0.0;
    for (long i = 0; i < n; i++)
    {
        sum += data[i];
    }

    free(data);

    /* TODO 3.4: Capturar final del programa */

    printf("N              : %ld\n", n);
    printf("Suma           : %.2f\n", sum);
    /* TODO 3.5: Imprimir mediciones */

    return 0;
}
