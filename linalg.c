#include "linalg.h"


/* the init function does not modify the matrix pointer.
 * the memory must be allocated in the calling code!
 */
int matrix_init(matrix *const m, int r, int c){
	int i,j;	  
	if(r<=0 || c<=0) return 0;
	m->rows=r; m->cols=c;

	if((m->data=(double**)malloc(r*sizeof(double*)))==NULL) return 0;
	for(i=0; i<r; i++){
		if((m->data[i]=(double*)malloc(c*sizeof(double)))==0) return 0;
		for(j=0; j<c; j++)
			m->data[i][j]=0.0;
	}
	return 1;
}

matrix* matrix_copy(const matrix* m){
	matrix *c;
	int i, j;

	c = (matrix*)malloc(sizeof(matrix));
	matrix_init(c, m->rows, m->cols);
	for(i=0; i<c->rows; i++)
		for(j=0; j<c->cols; j++)
			c->data[i][j]=m->data[i][j];

	return c;
}

void matrix_destroy(matrix *m){
	int i;
	for(i=0; i<m->rows; i++)
		free(m->data[i]);
	free(m->data);
}

void matrix_set_row(matrix* m, int r, ...){
	va_list ap;	  
	int i;
	double v;

	va_start(ap, r);
	for(i=0; i<m->cols; i++){
		v = (double)va_arg(ap, double);	  
		m->data[r-1][i] = v;
	}
	
	va_end(ap);
}

void matrix_set(matrix* m, int n, ...){
	va_list ap;	  
	int i;
	double v;

	va_start(ap, n);
	for(i=0; i<n; i++){
		v = (double)va_arg(ap, double);	  
		m->data[i/m->cols][i-(i/m->cols)*m->cols] = v;
	}
	
	va_end(ap);
}

matrix* matrix_multiply(const matrix *A, const matrix *B){
	matrix *C;
	int r, c, i;

	if(A->cols!=B->rows) return NULL;

	C=(matrix*)malloc(sizeof(matrix));
	matrix_init(C, A->rows, B->cols);
	/* C's entries are 0.0 */
	for(r=0; r<C->rows; r++){
		for(c=0; c<C->cols; c++){
			for(i=0; i<A->cols; i++){
				C->data[r][c]+=(double)(A->data[r][i]*B->data[i][c]);
			}
		}
	}
	return C;
}

matrix* matrix_transpose(const matrix *A){
	matrix *C;
	int r,c;

	C=(matrix*)malloc(sizeof(matrix));
	matrix_init(C, A->cols, A->rows);
	for(r=0; r<C->rows; r++)
		for(c=0; c<C->cols; c++)
			C->data[r][c]=A->data[c][r];

	return C;
}

/* solves A'Av=A'b */
matrix* matrix_solve_least_squares(const matrix *A, const matrix *b){
	matrix *At, *AtA, *Atb, *v, *Ab;
	int i, j;

	if(A->rows!=b->rows) return NULL;

	Ab = (matrix*)malloc(sizeof(matrix));
	v = (matrix*)malloc(sizeof(matrix));
	matrix_init(v, b->rows, 1);

	At = matrix_transpose(A);
	AtA=matrix_multiply(At, A);
	Atb=matrix_multiply(At, b);

	/* merge A and b to [A b] */
	matrix_init(Ab, A->rows, (A->cols)+1);
	for(i=0; i<A->rows; i++)
		for(j=0; j<A->cols; j++)
			Ab->data[i][j]=A->data[i][j];
	for(i=0; i<b->rows; i++)
		Ab->data[i][(Ab->cols)-1]=b->data[i][0];
	/* matrix_print("Ab in ls:", Ab);*/
	matrix_solve_gauss(Ab, v);

	matrix_destroy(At);		  
	matrix_destroy(AtA);		  
	matrix_destroy(Atb);		  
	matrix_destroy(Ab);		  
	return v;
}

matrix* matrix_solve_gauss(const matrix* m, matrix* v){
	int i, j, k;	  
	matrix *c;
	double *tmp;
	double a, b, r;

	/*
	 * 3 4 2 6 
	 * 2 6 4 1
	 * 6 7 1 2 
	 */
	if(m->rows!=v->rows) return NULL;
	c = matrix_copy(m);

	for(i=0; i<m->rows-1; i++){
		if(c->data[i][i]==0.0){
			/* swap rows below i */
			for(j=i+1; j<m->rows-1; j++){
				if(c->data[j][i]!=0.0){
					/* swap */	  
					tmp = c->data[j]; 
					c->data[j] = c->data[i];
					c->data[i] = tmp;
				}
			}
		}
		for(j=i+1; j<c->rows; j++){
			/* xa + b = 0 
			 * x = -b/a */
			a = c->data[i][i];		  
			b = c->data[j][i];
			for(k=i; k<c->cols; k++)
				c->data[j][k] += c->data[i][k]*(-b/a);
		}
	}

	/* normalize triangular */
	for(i=0; i<c->rows; i++){
		r = c->data[i][i];	  
		for(j=i; j<c->cols; j++)
			c->data[i][j] /= r;
	}
	
	/* fill solution vector */
	for(i=v->rows-1; i>=0; i--){
		v->data[i][0]=c->data[i][c->cols-1];
		for(j=c->cols-2; j>i; j--)
			v->data[i][0] -= v->data[j][0]*c->data[i][j];
	}
	
	return c;
}

void matrix_print(const char* label, const matrix* m){
	int i,j;
	fprintf(stderr, "%s = \n", label);
	for(i=0; i<m->rows; i++){
		for(j=0; j<m->cols; j++){
			fprintf(stderr, "%.*f\t", PRINT_MATRIX_PRECISION, m->data[i][j]);
		}
		fputc('\n', stderr);
	}
	return;
}

double calculate_distance(double x1, double y1, double x2, double y2){
	double dist;
	return dist;
}
