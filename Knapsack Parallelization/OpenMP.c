#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define max(a, b) ((a > b) ? a : b)

int knapSack(int W, int *wt, int *val, int n) {
    int *K = (int *)malloc((n + 1) * (W + 1) * sizeof(int));

    #pragma omp parallel for
    for (int i = 0; i <= n; i++) {
        for (int w = 0; w <= W; w++) {
            if (i == 0 || w == 0)
                K[i * (W + 1) + w] = 0;
            else if (wt[i - 1] <= w)
                K[i * (W + 1) + w] = max(val[i - 1] + K[(i - 1) * (W + 1) + w - wt[i - 1]],
                                          K[(i - 1) * (W + 1) + w]);
            else
                K[i * (W + 1) + w] = K[(i - 1) * (W + 1) + w];
        }
    }

    int result = K[n * (W + 1) + W];

    free(K);
    return result;
}

int main() {
    clock_t start, end;
    srand(time(NULL));
    int *profit, *weight;
    int n = 10000;
    profit = (int *)malloc(n * sizeof(int));
    weight = (int *)malloc(n * sizeof(int));

    if (profit == NULL || weight == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }

    int W = 10000;
    for (int i = 0; i < 10000; i++) {
        profit[i] = rand() % 5000 + 1;
        weight[i] = rand() % 5000 + 1;
    }

    start = clock();
    printf("%d", knapSack(W, weight, profit, n));
    end = clock();
    double elapsed_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("\n");
    printf("Execution time: %f seconds\n", elapsed_time);

    free(profit);
    free(weight);

    return 0;
}
