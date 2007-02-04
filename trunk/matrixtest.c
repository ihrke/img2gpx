#include "linalg.h"


int main(){
	matrix m, v, *s, *t, *t2;
	s=(matrix*)malloc(sizeof(matrix));
	t=(matrix*)malloc(sizeof(matrix));
	t2=(matrix*)malloc(sizeof(matrix));
	matrix_init(t2, 4, 2);
	matrix_set(t2, 8, 1.0, 5.0,
						 	2.0, 6.0,
							3.0, 7.0,
							4.0, 8.0);
	matrix_init(&m, 3, 4);
	matrix_init(&v, 3, 1);
	matrix_print("m",(const matrix*)&m);
	matrix_set(&m, 12, 1.0, 1.0, 1.0, 1.0, 
						    2.0, 2.0, 2.0, 2.0, 
							 1.0, 1.0, 1.0, 1.0);
	matrix_print("m", (const matrix*)&m);

	if((s = matrix_solve_gauss((const matrix*)&m, &v))==NULL){
		printf("Matrix has no solution");		  
		return 0;
	};
	matrix_print("s",(const matrix*)s);
	matrix_print("v",(const matrix*)&v);
	t= matrix_multiply((const matrix*)&m, (const matrix*)t2);
	matrix_print("t2",t2);
	matrix_print("m*t2",(const matrix*) t);
	matrix_print("m'", matrix_transpose(&m));
	return 0;
}
