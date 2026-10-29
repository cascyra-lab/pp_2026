#include <stdio.h>
#include <thread>
#include <chrono>
#include "CycleTimer.h"

typedef struct {
    float x0, x1;
    float y0, y1;
    unsigned int width;
    unsigned int height;
    int maxIterations;
    int* output;
    int threadId;
    int numThreads;
} WorkerArgs;

extern void mandelbrotSerial(
    float x0, float y0, float x1, float y1,
    int width, int height,
    int startRow, int numRows,
    int maxIterations,
    int output[]);

// Внутренняя функция mandel (точно как в оригинале)
static inline int mandel(float c_re, float c_im, int count)
{
    float z_re = c_re, z_im = c_im;
    int i;
    for (i = 0; i < count; ++i) {
        if (z_re * z_re + z_im * z_im > 4.f)
            break;

        float new_re = z_re * z_re - z_im * z_im;
        float new_im = 2.f * z_re * z_im;
        z_re = c_re + new_re;
        z_im = c_im + new_im;
    }
    return i;
}

// ПУНКТ 4: Чередующееся распределение строк (interleaved assignment)
void workerThreadStart(WorkerArgs *const args) {
    auto t0 = std::chrono::high_resolution_clock::now();

    // Предварительно вычисляем шаги (точно как в mandelbrotSerial)
    float dx = (args->x1 - args->x0) / (float)args->width;
    float dy = (args->y1 - args->y0) / (float)args->height;

    // Чередующееся распределение: каждый поток берет строки с шагом numThreads
    for (int y = args->threadId; y < (int)args->height; y += args->numThreads) {
        for (int x = 0; x < (int)args->width; ++x) {
            // Точно такая же формула, как в mandelbrotSerial
            float px = args->x0 + x * dx;
            float py = args->y0 + y * dy;

            // Используем ту же функцию mandel
            args->output[y * (int)args->width + x] = mandel(px, py, args->maxIterations);
        }
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    printf("Thread %d finished in %.2f ms\n", args->threadId, ms);
}

void mandelbrotThread(
    int numThreads,
    float x0, float y0, float x1, float y1,
    int width, int height,
    int maxIterations,
    int output[])
{
    WorkerArgs* argsArray = new WorkerArgs[numThreads];
    std::thread* threads = new std::thread[numThreads];

    for (int i = 0; i < numThreads; i++) {
        argsArray[i].x0 = x0;
        argsArray[i].x1 = x1;
        argsArray[i].y0 = y0;
        argsArray[i].y1 = y1;
        argsArray[i].width = (unsigned int)width;
        argsArray[i].height = (unsigned int)height;
        argsArray[i].maxIterations = maxIterations;
        argsArray[i].numThreads = numThreads;
        argsArray[i].threadId = i;
        argsArray[i].output = output;

        threads[i] = std::thread(workerThreadStart, &argsArray[i]);
    }

    for (int i = 0; i < numThreads; i++) {
        threads[i].join();
    }

    delete[] threads;
    delete[] argsArray;
}