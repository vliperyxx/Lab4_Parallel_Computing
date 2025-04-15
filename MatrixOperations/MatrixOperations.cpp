#include <iostream>
#include <cstdlib>
#include <ctime>
#include <iomanip>
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

void MatrixOperations::printMatrix(int* matrix, int size) {
    for (int row = 0; row < std::min(size, 10); row++) {
        for (int col = 0; col < std::min(size, 10); col++) {
            std::cout << std::setw(6) << matrix[row * size + col] << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}