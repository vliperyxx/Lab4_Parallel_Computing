#include <iostream>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <thread>
#include <vector>
#include "MatrixOperations.h"

int* MatrixOperations::allocateMatrix(int size) {
    return new int[size * size];
}

void MatrixOperations::freeMatrix(int* matrix) {
    delete[] matrix;
}

void MatrixOperations::fillMatrix(int* matrix, int size) {
    srand(time(nullptr));

    for (int row = 0; row < size; row++) {
        for (int col = 0; col < size; col++) {
            matrix[row * size + col] = 1 + rand() % 10000;
        }
    }
}

void MatrixOperations::calculateMaxOfColumn(int* matrix, int size, int col) {
    if (col >= size)
        return;

    int maxRow = col;
    int maxVal = matrix[col * size + col];

    for (int row = 0; row < size; row++) {
        if (matrix[row * size + col] > maxVal) {
            maxVal = matrix[row * size + col];
            maxRow = row;
        }
    }

    if (maxRow != col) {
        std::swap(matrix[col * size + col], matrix[maxRow * size + col]);
    }
}

void MatrixOperations::calculateMaxMultiThread(int* matrix, int size, int threadCount, std::atomic<int>* progressTracker) {
    std::vector<std::thread> threads(threadCount);

    for (int threadId = 0; threadId < threadCount; threadId++) {
        threads[threadId] = std::thread(calculateMaxForColumns,matrix, size, threadId, threadCount, progressTracker);
    }

    for (auto& thread : threads) {
        thread.join();
    }
}

void MatrixOperations::calculateMaxForColumns(int* matrix, int size, int threadId, int threadNum, std::atomic<int>* progressTracker) {
    for (int col = threadId; col < size; col += threadNum) {
        calculateMaxOfColumn(matrix, size, col);
        if (progressTracker) {
            (*progressTracker)++;
        }
    }
}

void MatrixOperations::printMatrix(int* matrix, int size) {
    for (int row = 0; row < std::min(size, 10); row++) {
        for (int col = 0; col < std::min(size, 10); col++) {
            std::cout << std::setw(6) << matrix[row * size + col] << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}