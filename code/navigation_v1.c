#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/resource.h>

static void bubble_sort(double *arr, int n)
{
    for (int i = 0; i < n - 1; i++)
    {
        for (int j = 0; j < n - 1 - i; j++)
        {
            if (arr[j] > arr[j + 1])
            {
                double tmp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = tmp;
            }
        }
    }
}

int main(void)
{
    int n;

    if (scanf("%d", &n) != 1 || n <= 0)
    {
        fprintf(stderr, "Error: cantidad invalida de segmentos\n");
        return 1;
    }

    double *costs = malloc((size_t)n * sizeof(double));
    if (costs == NULL)
    {
        fprintf(stderr, "Error: no se pudo reservar memoria\n");
        return 1;
    }

    for (int i = 0; i < n; i++)
    {
        if (scanf("%lf", &costs[i]) != 1)
        {
            fprintf(stderr, "Error: no se pudo leer el costo %d\n", i + 1);
            free(costs);
            return 1;
        }
    }

    /* TODO 5.1: Variables de medición */
    /*
    struct timespec t0, t1;
    struct rusage   usage;
    */

    /* TODO 5.2: Captura de inicio */
    /*
    clock_gettime(CLOCK_MONOTONIC, &t0);
    */

    bubble_sort(costs, n);

    /* TODO 5.3: Captura de final y reporte */
    /*
    clock_gettime(CLOCK_MONOTONIC, &t1);
    getrusage(RUSAGE_SELF, &usage);
    fprintf(stderr, "Tiempo de ordenamiento : %.6f s\n",
            (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec) * 1e-9);
    fprintf(stderr, "Memoria RSS            : %ld KB\n", usage.ru_maxrss);
    */

    double total = 0.0;
    for (int i = 0; i < n; i++)
    {
        total += costs[i];
    }

    printf("Segmentos procesados : %d\n", n);
    printf("Costo min            : %.2f\n", costs[0]);
    printf("Costo max            : %.2f\n", costs[n - 1]);
    printf("Costo promedio       : %.2f\n", total / (double)n);
    printf("Costo total          : %.2f\n", total);

    free(costs);
    return 0;
}
