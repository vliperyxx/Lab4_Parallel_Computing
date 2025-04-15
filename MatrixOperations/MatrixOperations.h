#pragma once
#include <atomic>

class MatrixOperations {
public:
    static int* allocateMatrix(int size);
    static void freeMatrix(int* matrix);
    static void fillMatrix(int* matrix, int size);
    static void printMatrix(int* matrix, int size);
    static void calculateMaxOfColumn(int* matrix, int size, int col);
    static void calculateMaxMultiThread(int* matrix, int size, int threadCount, std::atomic<int>* progressTracker = nullptr);
    static void calculateMaxForColumns(int* matrix, int size, int threadId, int threadNum, std::atomic<int>* progressTracker = nullptr);
};
