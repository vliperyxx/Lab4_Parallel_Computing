#pragma once

class MatrixOperations {
public:
    static int* allocateMatrix(int size);
    static void freeMatrix(int* matrix);
    static void fillMatrix(int* matrix, int size);
    static void printMatrix(int* matrix, int size);
};
