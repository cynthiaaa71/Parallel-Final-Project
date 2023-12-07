#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <sys/time.h>
#include <time.h>

void init_things(int n, int weight[n], int profit[n])
{
    int i = 0;

    for (i = 0; i < n; i++)
    {
        weight[i] = (random() % 10000) + 1;
        profit[i] = (random() % 10000) + 1;
    }
}
// A utility function that returns
// maximum of two integers
int max(int a, int b) { return (a > b) ? a : b; }

// subblock size
#define ROW 64
#define COL 512

int min(int i, int j)
{
    return (i < j) ? i : j;
}

void knapsack(int n, int c, int rows, int weight[rows], int profit[rows],
            int start, int rank, int size)
{
    int recv_rank = (rank - 1) % size; // rank to receive data
    int send_rank = (rank + 1) % size; // rank to send data
    if (start == 0)
    { // deal with first block, since it doesn't receive data from any nodes
        int total[rows][c];
        int i, j;

        for (j = 0; j < c; j += COL)
        {
            int cols = min(COL, c - j);
            int k;
            for (k = j; k < j + cols; k++)
            {
                if (weight[0] > k)
                {
                    total[0][k] = 0;
                }
                else
                {
                    total[0][k] = profit[0];
                }
            }
            // compute subblock
            for (i = 1; i < rows; i++)
            {
                for (k = j; k < j + cols; k++)
                {
                    // int ni = i+start;
                    if (weight[i] > k)
                    {
                        total[i][k] = total[i - 1][k];
                    }
                    else
                    {
                        total[i][k] = max(total[i - 1][k], total[i - 1][k - weight[i]] + profit[i]);
                    }
                }
            }
            // send last row to next node
            MPI_Send(&total[rows - 1][j], cols, MPI_INT, send_rank, j, MPI_COMM_WORLD);
        }
    }
    else
    {
        int total[rows + 1][c]; // use the first row to store the data from last node
        int i, j;
        for (j = 0; j < c; j += COL)
        {
            int cols = min(COL, c - j);
            int k;
            // receive data from last node
            MPI_Recv(&total[0][j], cols, MPI_INT, recv_rank, j, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            for (i = 1; i <= rows; i++)
            {
                for (k = j; k < j + cols; k++)
                {
                    int ni = i - 1; // ni is index for weight and profit, notice the first row is the data from last node
                    if ((k < weight[ni]) ||
                        (total[i - 1][k] >= total[i - 1][k - weight[ni]] + profit[ni]))
                    {
                        total[i][k] = total[i - 1][k];
                    }
                    else
                    {
                        total[i][k] = total[i - 1][k - weight[ni]] + profit[ni];
                    }
                }
            }

            if (start + rows == n && j + cols == c)
            {
                // computer the last subblock of last ROW, print the final result
                printf("max profit: %d \n", total[rows][c - 1]);
            }
            else if (start + rows != n)
            {
                // if it is not last ROW, we need send data to next node.
                MPI_Send(&total[rows][j], cols, MPI_INT, send_rank, j, MPI_COMM_WORLD);
            }
        }
    }
}

int main(int argc, char *argv[])
{

    int i, n = 0, W = 0;
    clock_t start, end, total_start, total_end;
    total_start= CLOCK();
    srand(time(NULL));
    //	int *weights, *profits;

    int(*table)[W], (*flags)[W];

    if (argc > 2 && argc < 5)
    {
        n = atoi(argv[1]); /* Number of items */
        W = atoi(argv[2]); /* Total Weight */
    }
    if ((n <= 0) || (W <= 0))
    {
        fprintf(stdout, "usage %s n c - where n and c >= 0\n", argv[0]);
        exit(1);
    }

    int size, rank;
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    start = clock();
    for (i = 0; i < n; i += ROW)
    {
        int rows = min(ROW, n - i);
        int *weights = malloc(rows * sizeof(int)); // initial weights locally
        int *profits = malloc(rows * sizeof(int)); // initial weights locally
        init_things(rows, weights, profits);
        if ((i / ROW) % size == rank)
            knapsack(n, W, rows, weights, profits, i, rank, size); // solve subblock
        free(weights);
        free(profits);
    }
    end = clock();

    double elapsed_time = ((double)(end - start)) / CLOCKS_PER_SEC;

    if (rank == 0)
    {
        printf("\n");
        printf("parallel Execution time: %f seconds\n", elapsed_time);
    }

    MPI_Finalize();
    total_end=CLOCK();
    double totalElapsed_time = ((double)(total_end - total_start)) / CLOCKS_PER_SEC;
    printf("total Execution time: %f seconds\n", totalElapsed_time);
    return 0;
}