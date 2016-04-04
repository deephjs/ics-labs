/*******************************************************************
 * name: Huang Junsen
 * ID: 5130379024
 * ****************************************************************/

/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
	int n1,n2,n3,n4,n5,n6,n7,n8;
	int i,j,k,l;
	if(M==32 && N==32){
		for(i = 0; i < 4; ++i){
			for(j = 0; j < 4; ++j){
				for(k = 8*i; k < 8*i+8; ++k){
					n1 = A[k][8*j];
					n2 = A[k][8*j + 1];
					n3 = A[k][8*j + 2];
					n4 = A[k][8*j + 3];
					n5 = A[k][8*j + 4];
					n6 = A[k][8*j + 5];
					n7 = A[k][8*j + 6];
					n8 = A[k][8*j + 7];
					B[8*j][k] = n1;
					B[8*j+1][k] = n2;
					B[8*j+2][k] = n3;
					B[8*j+3][k] = n4;
					B[8*j+4][k] = n5;
					B[8*j+5][k] = n6;
					B[8*j+6][k] = n7;
					B[8*j+7][k] = n8;
				}
			}
		}
	}
	else if(M==64 && N==64){
		for(i = 0; i < 8; ++i){
			for(j = 0; j < 8; ++j){
				for(k = 8*i; k < 8*i+4; ++k){
					n1 = A[k][8*j];
					n2 = A[k][8*j+1];
					n3 = A[k][8*j+2];
					n4 = A[k][8*j+3];
					B[8*j][k] = n1;
					B[8*j+1][k] = n2;
					B[8*j+2][k] = n3;
					B[8*j+3][k] = n4;
				}
				if(i != j){
					for(k = 8*j+4; k < 8*j+8; ++k){
						for(l = 8*i; l < 8*i+4; ++l){
							B[k-4][l+4] = A[l][k];
						}
					}
				}
				else{
					for(k = 0; k < 4; ++k){
						n1 = A[8*i+k][8*j+4];
						n2 = A[8*i+k][8*j+5];
						n3 = A[8*i+k][8*j+6];
						n4 = A[8*i+k][8*j+7];
						B[8*j][8*i+4+k] = n1;
						B[8*j+1][8*i+4+k] = n2;
						B[8*j+2][8*i+4+k] = n3;
						B[8*j+3][8*i+4+k] = n4;
					}
				}
				for(k = 8*j; k < 8*j+4; ++k){
					n1 = B[k][8*i+4];
					n2 = B[k][8*i+5];
					n3 = B[k][8*i+6];
					n4 = B[k][8*i+7];
					for(l = 8*i+4; l < 8*i+8; ++l){
						B[k][l] = A[l][k];	
					}
					B[k+4][8*i] = n1;
					B[k+4][8*i+1] = n2;
					B[k+4][8*i+2] = n3;
					B[k+4][8*i+3] = n4;
				}
				for(k = 8*i+4; k < 8*i+8; ++k){
					n1 = A[k][8*j+4];
					n2 = A[k][8*j+5];
					n3 = A[k][8*j+6];
					n4 = A[k][8*j+7];
					B[8*j+4][k] = n1;
					B[8*j+5][k] = n2;
					B[8*j+6][k] = n3;
					B[8*j+7][k] = n4;
				}
			}
		}
	}
	else if(M == 61 && N == 67){
		for(i = 0; i < 5; ++i){
			for(j = 0; j < 4; ++j){
				for(k = 16*i; k < 16*i+16; ++k){
					for(l = 16*j; l < 16*j+16; ++l)
						if(k<67 && l < 61) 
							B[l][k] = A[k][l];
				}
			}
		}
	}
	else {
		for (i = 0; i < N; i++){
			for (j = 0; j < M; j++){
				B[j][i] = A[i][j];
			}
		}
	}
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    
}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

