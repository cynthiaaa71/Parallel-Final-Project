#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define max(a, b) ((a > b) ? a : b)

__global__ void knapSackKernel(int W, int *wt, int *val, int *K, int n) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;

    if (i < n + 1) {
        for (int w = 0; w < W + 1; w++) {
            if (i == 0 || w == 0)
                K[i * (W + 1) + w] = 0;
            else if (wt[i - 1] <= w)
                K[i * (W + 1) + w] = max(val[i - 1] + K[(i - 1) * (W + 1) + w - wt[i - 1]],
                                          K[(i - 1) * (W + 1) + w]);
            else
                K[i * (W + 1) + w] = K[(i - 1) * (W + 1) + w];
        }
    }
}

int knapSack(int W, int *wt, int *val, int n) {
    int *K, *d_wt, *d_val, *d_K;
    int size = (n + 1) * (W + 1) * sizeof(int);

    K = (int *)malloc(size);

    // Allocate memory for wt, val, and K on the GPU side
    cudaMalloc((void **)&d_wt, n * sizeof(int));
    cudaMalloc((void **)&d_val, n * sizeof(int));
    cudaMalloc((void **)&d_K, size);

    cudaMemcpy(d_wt, wt, n * sizeof(int), cudaMemcpyHostToDevice);
    cudaMemcpy(d_val, val, n * sizeof(int), cudaMemcpyHostToDevice);

    int threadsPerBlock = 256;
    int blocksPerGrid = (n + threadsPerBlock - 1) / threadsPerBlock;

    knapSackKernel<<<blocksPerGrid, threadsPerBlock>>>(W, d_wt, d_val, d_K, n);
    
    cudaMemcpy(K, d_K, size, cudaMemcpyDeviceToHost);

    int result = K[n * (W + 1) + W];

    free(K);
    cudaFree(d_wt);
    cudaFree(d_val);
    cudaFree(d_K);

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
    for (int i = 0; i < n; i++) {
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