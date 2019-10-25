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
    if (M == 32 )
    {  
        for (int i = 0; i < M; i+=8)
        {
            for (int  j = 0; j < N; j+=8)
            {
                for (int ii = i; ii < i+8; ii++)
                {
                    int t0=A[ii][j+0];
                    int t1=A[ii][j+1];
                    int t2=A[ii][j+2];
                    int t3=A[ii][j+3];
                    int t4=A[ii][j+4];
                    int t5=A[ii][j+5];
                    int t6=A[ii][j+6];
                    int t7=A[ii][j+7];

                    B[j+7][ii]=t7;
                    B[j+6][ii]=t6;
                    B[j+5][ii]=t5;
                    B[j+4][ii]=t4;
                    B[j+3][ii]=t3;
                    B[j+2][ii]=t2;
                    B[j+1][ii]=t1;
                    B[j+0][ii]=t0;
                    
                }
                
            }
            
        }
        
    }
    else if (M == 64)
    {
        for (int i = 0; i < M ; i+=4)
        {
            for (int j = 0; j < N; j+=4)
            {
                for (int ii = i; ii <i+4; ii++)
                {
                    int t0=A[ii][j+0];
                    int t1=A[ii][j+1];
                    int t2=A[ii][j+2];
                    int t3=A[ii][j+3];
                    B[j+3][ii]=t3;
                    B[j+2][ii]=t2;
                    B[j+1][ii]=t1;
                    B[j+0][ii]=t0;
                }
                
            }
            
        }
        
    }
    else
     {
        //16*16 blocking
        for (int i = 0; i < 48; i+=16)
        {
            for (int j = 0; j < 64; j+=16)
            {
                for (int k = i; k < i+16; k++)
                {
                    int t1=A[k][j];
                    int t2=A[k][j+1];
                    int t3=A[k][j+2];
                    int t4=A[k][j+3];
                    int t5=A[k][j+4];
                    int t6=A[k][j+5];
                    int t7=A[k][j+6];
                    int t8=A[k][j+7];

                    B[j+7][k]=t8;
                    B[j+6][k]=t7;
                    B[j+5][k]=t6;
                    B[j+4][k]=t5;
                    B[j+3][k]=t4;
                    B[j+2][k]=t3;
                    B[j+1][k]=t2;
                    B[j][k]=t1;

                    t1=A[k][j+8];
                    t2=A[k][j+9];  
                    t3=A[k][j+10];
                    t4=A[k][j+11];
                    t5=A[k][j+12];
                    t6=A[k][j+13];
                    t7=A[k][j+14];
                    t8=A[k][j+15];   

                    B[j+15][k]=t8;
                    B[j+14][k]=t7;
                    B[j+13][k]=t6;
                    B[j+12][k]=t5;
                    B[j+11][k]=t4;
                    B[j+10][k]=t3;
                    B[j+9][k]=t2;
                    B[j+8][k]=t1;
                            
                }
                
            }
            
        }

        for(int i=48;i<61;i++)
        {
            for(int j=0;j<67;j++)
            {
                B[j][i]=A[i][j];
            }
        }
                for(int i=0;i<48;i++)
        {
            for (int j = 64; j < 67; j++)
            {
                B[j][i]=A[i][j];
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

