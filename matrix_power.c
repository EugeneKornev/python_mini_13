#define PY_SIZE_T_CLEAN
#include <Python.h>
#include <stdlib.h>

void* nc_malloc(size_t size) {
    void* result = malloc(size);
    if (result == NULL) {
        exit(1);
    }
    return result;
}


static void matrix_mult(size_t n, double** matr1, double** matr2, double** result) {
    for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < n; j++) {
            result[i][j] = 0;
            for (size_t k = 0; k < n; k++) {
                result[i][j] += matr1[i][k] * matr2[k][j];
            }
        }
    }
    return;
}

static PyObject* eye(size_t n) {
    PyObject* res = PyList_New(n);
    for (size_t i = 0; i < n; i++) {
        PyObject* row = PyList_New(n);
        for (size_t j = 0; j < n; j++) {
            PyList_SetItem(row, j, PyFloat_FromDouble((i == j) ? 1 : 0));
        }
        PyList_SetItem(res, i, row);
    }
    return res;
}

static PyObject* foreign_matrix_power(PyObject* self, PyObject* args) {
    PyObject* matrix;
    int power;
    if (!PyArg_ParseTuple(args, "Oi", &matrix, &power)) {
        return NULL;
    }
    if (!PyList_Check(matrix) || power < 0) {
        return NULL;
    }
    size_t N = (size_t) PyList_Size(matrix);
    if (N == 0) {
        return NULL;
    }
    for (size_t i = 0; i < N; i++) {
        PyObject* row = PyList_GetItem(matrix, i);
        if (!PyList_Check(row) || PyList_Size(row) != N) {
            return NULL;
        }
    }
    if (power == 1) {
        Py_INCREF(matrix);
        return matrix;
    }
    if (power == 0) {
        PyObject* result = eye(N);
        return result;
    }
    double** matr = (double**) nc_malloc(N * sizeof(double*));
    double** result = (double**) nc_malloc(N * sizeof(double*));
    double** buffer = (double**) nc_malloc(N * sizeof(double*));
    for (size_t p = 0; p < N; p++) {
        matr[p] = (double*) nc_malloc(N * sizeof(double));
        result[p] = (double*) nc_malloc(N * sizeof(double));
        buffer[p] = (double*) nc_malloc(N * sizeof(double));
    }
    double** tmp;
    for (size_t j = 0; j < N; j++) {
        PyObject* row = PyList_GetItem(matrix, j);
        for (size_t k = 0; k < N; k++) {
            double cur = PyFloat_AsDouble(PyList_GetItem(row, k));
            matr[j][k] = cur;
            result[j][k] = cur;
            buffer[j][k] = cur;
        }
    }
    for (size_t l = 0; l < power - 1; l++) {
        matrix_mult(N, matr, result, buffer);
        tmp = (tmp = buffer, buffer = result, result = tmp);
    }
    PyObject* py_result = PyList_New(N);
    for (size_t a = 0; a < N; a++) {
        PyObject* row = PyList_New(N);
        for (size_t b = 0; b < N; b++) {
            PyList_SetItem(row, b, PyFloat_FromDouble(result[a][b]));
        }
        PyList_SetItem(py_result, a, row);
    }
    for (size_t c = 0; c < N; c++) {
        free(matr[c]);
        free(result[c]);
        free(buffer[c]);
    }
    free(matr);
    free(result);
    free(buffer);
    return py_result;
}

static PyMethodDef ForeignMethods[] = {
    {"foreign_matrix_power", foreign_matrix_power, METH_VARARGS, "Exponentiation of the matrix"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef foreignmodule = {
    PyModuleDef_HEAD_INIT, "matrix", NULL, -1, ForeignMethods
};

PyMODINIT_FUNC PyInit_matrix(void) {
    return PyModule_Create(&foreignmodule);
}