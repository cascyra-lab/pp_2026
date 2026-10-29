#include <stdio.h>
#include <stdlib.h>
#include <cuda_runtime.h>
#include <time.h>

__global__ void matMulKernel(const float *A, const float *B, float *C, int N) {
    int row = blockIdx.y * blockDim.y + threadIdx.y;
    int col = blockIdx.x * blockDim.x + threadIdx.x;

    if (row < N && col < N) {
        float sum = 0.0f;
        for (int k = 0; k < N; ++k) {
            sum += A[row * N + k] * B[k * N + col];
        }
        C[row * N + col] = sum;
    }
}

int main() {
    int sizes[] = {512, 1024, 2048};
    int block_configs[][2] = {{16, 16}, {32, 32}};

    printf("N,BlockX,BlockY,Time_sec,GFLOPS\n");

    for (int s = 0; s < 3; ++s) {
        int N = sizes[s];
        size_t size = N * N * sizeof(float);

        float *h_A = (float*)malloc(size);
        float *h_B = (float*)malloc(size);
        float *h_C = (float*)malloc(size);

        srand(time(NULL));
        for (int i = 0; i < N * N; ++i) {
            h_A[i] = (float)rand() / RAND_MAX;
            h_B[i] = (float)rand() / RAND_MAX;
        }

        float *d_A, *d_B, *d_C;
        cudaMalloc(&d_A, size);
        cudaMalloc(&d_B, size);
        cudaMalloc(&d_C, size);

        cudaMemcpy(d_A, h_A, size, cudaMemcpyHostToDevice);
        cudaMemcpy(d_B, h_B, size, cudaMemcpyHostToDevice);

        for (int b = 0; b < 2; ++b) {
            int bx = block_configs[b][0];
            int by = block_configs[b][1];

            dim3 block(bx, by);
            dim3 grid((N + bx - 1) / bx, (N + by - 1) / by);

            cudaEvent_t start, stop;
            cudaEventCreate(&start);
            cudaEventCreate(&stop);

            cudaEventRecord(start);
            matMulKernel<<<grid, block>>>(d_A, d_B, d_C, N);
            cudaEventRecord(stop);
            cudaEventSynchronize(stop);

            float milliseconds = 0;
            cudaEventElapsedTime(&milliseconds, start, stop);
            double time_sec = milliseconds / 1000.0;

            double gflops = (2.0 * N * N * N) / (time_sec * 1e9);

            printf("%d,%d,%d,%.6f,%.2f\n", N, bx, by, time_sec, gflops);

            cudaEventDestroy(start);
            cudaEventDestroy(stop);
        }

        cudaFree(d_A);
        cudaFree(d_B);
        cudaFree(d_C);
        free(h_A);
        free(h_B);
        free(h_C);
    }

    return 0;
}