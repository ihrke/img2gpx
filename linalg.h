#ifndef LINALG_H
#define LINALG_H

#include<stdio.h>
#include<stdlib.h>
#include<stdarg.h>
#include"wplist.h"

#define PRINT_MATRIX_PRECISION 2

typedef struct{
	int rows, cols;
	double **data;
} matrix;

/* the init function does not modify the matrix pointer.
 * the memory must be allocated in the calling code!
 */
int matrix_init(matrix *const m, int r, int c);

/* return NULL is system has no solution
 * returns triangular matrix else
 * (v is result vector)
 */

/* v is the solution vector, the returned matrix is a normalized triangular matrix */
matrix* matrix_solve_gauss(const matrix* m, matrix* v);

/* solves A'Av=A'b 
 * returning the solution vector v*/
matrix* matrix_solve_least_squares(const matrix* A, const matrix *B);

void matrix_print(const char* label,const matrix* m);
void matrix_destroy(matrix *m);
matrix* matrix_copy(const matrix* m);
void matrix_set(matrix* m, int n, ...);
void matrix_set_row(matrix* m, int r, ...);
matrix* matrix_multiply(const matrix* A, const matrix* B);
matrix* matrix_transpose(const matrix *A);

double calculate_distance(double x1, double y1, double x2, double y2);
#endif
