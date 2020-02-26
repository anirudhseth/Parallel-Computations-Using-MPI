/* Reaction-diffusion equation in 1D
 * Solution by Jacobi iteration
 * simple MPI implementation
 *
 * C Michael Hanke 2006-12-12
 */

#define MIN(a, b)((a) < (b) ? (a) : (b))

/* Use MPI */
#include "mpi.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

/* define problem to be solved */
# define N 1000 /* number of inner grid points */ 
# define SMX 1000000 /* number of iterations */

/* implement coefficient functions */
extern double r(const double x);
extern double f(const double x);

double r(const double x) {
  return -x;
}

double f(const double x) {
  return 2 - x * x * (x - 1);
}

/* We assume linear data distribution. The formulae according to the lecture
   are:
      L = N/P;
      R = N%P;
      I = (N+P-p-1)/P;    (number of local elements)
      n = p*L+MIN(p,R)+i; (global index for given (p,i)
   Attention: We use a small trick for introducing the boundary conditions:
      - The first ghost point on p = 0 holds u(0);
      - the last ghost point on p = P-1 holds u(1).
   Hence, all local vectors hold I elements while u has I+2 elements.
*/
/* returns global index for a given p and i */
int getGlobalIndex(int L, int R, int p, int i) {
  return p * L + MIN(R, p) + i;
}
int main(int argc, char * argv[]) {
  /* local variable */
  int P, p, M, L, R, I, tag, step;
  double h;
  /* Initialize MPI */
  MPI_Status status;
  MPI_Init( &argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &P); /* Number of processors*/
  MPI_Comm_rank(MPI_COMM_WORLD, &p); /* Current processors*/
  if (N < P) {
    fprintf(stdout, "Too few discretization points...\n");
    exit(1);
  }
  tag = 100;
  /* Compute local indices for data distribution */
  L = N / P;
  R = N % P;
  I = p < R ? L + 1 : L;
  h = 1.0 / (N + 1);

  /* arrays */
  double *unew = (double * ) calloc(I, sizeof(double));
  double *rr = (double * ) calloc(I, sizeof(double));
  double *ff = (double * ) calloc(I, sizeof(double));
  double *u = (double * ) calloc(I + 2, sizeof(double));

  for (int i = 0; i < I; i++) {
    int z = getGlobalIndex(L, R, p, i);
    double xn = z * h;
    ff[i] = f(xn);
    rr[i] = r(xn);
  }

  /* Jacobi iteration */
  for (step = 0; step < SMX; step++) {
    /* RB communication of overlap */
    if (p % 2 == 0) { // red
      if (p != P - 1) { // check if no processor on the right
        MPI_Send(&u[I], 1, MPI_DOUBLE, p + 1, tag, MPI_COMM_WORLD);
        MPI_Recv(&u[I + 1], 1, MPI_DOUBLE, p + 1, tag, MPI_COMM_WORLD,&status);
      }
      if (p != 0) { // check if no processor on the left
        MPI_Send(&u[1], 1, MPI_DOUBLE, p - 1, tag, MPI_COMM_WORLD);
        MPI_Recv(&u[0], 1, MPI_DOUBLE, p - 1, tag, MPI_COMM_WORLD,&status);
      }
    } else { // black
      if (p != 0) {
        MPI_Recv(&u[0], 1, MPI_DOUBLE, p - 1, tag, MPI_COMM_WORLD,&status);
        MPI_Send(&u[1], 1, MPI_DOUBLE, p - 1, tag, MPI_COMM_WORLD);
      }
      if (p != P - 1) {
        MPI_Recv(&u[I + 1], 1, MPI_DOUBLE, p + 1, tag, MPI_COMM_WORLD,&status);
        MPI_Send(&u[I], 1, MPI_DOUBLE, p + 1, tag, MPI_COMM_WORLD);
      }
    }
    /* local iteration step */
    for (int i = 0; i < I; i++) {
      unew[i] = (u[i] + u[i + 2] - h * h * ff[i]) / (2.0 - h * h * rr[i]);
    }
    for (int i = 0; i < I; i++) {
      u[i + 1] = unew[i];
    }

  }

  /* output for graphical representation */
  /* Instead of using gather (which may lead to excessive memory requirements
   on the master process) each process will write its own data portion. This
   introduces a sequentialization: the hard disk can only write (efficiently)
   sequentially. Therefore, we use the following strategy:
   1. The master process writes its portion. (file creation)
   2. The master sends a signal to process 1 to start writing.
   3. Process p waites for the signal from process p-1 to arrive.
   4. Process p writes its portion to disk. (append to file)
   5. process p sends the signal to process p+1 (if it exists).
*/
  const char * filename = "output.txt";
  double writeSignal = 1.0;
  FILE * f;
  if (p == 0) {
    f = fopen(filename, "w");
    for (size_t i = 0; i < I; i++) {
      fprintf(f, "%f ", unew[i]);
    }
    fclose(f);
    MPI_Send(&writeSignal, 1, MPI_DOUBLE, p + 1, tag, MPI_COMM_WORLD);
  } else {
    MPI_Recv(&writeSignal, 1, MPI_DOUBLE, p - 1, tag, MPI_COMM_WORLD, &status);
    f = fopen(filename, "a");
    for (size_t i = 0; i < I; i++) {
      fprintf(f, "%f ", unew[i]);
    }
    fclose(f);
    if (p != P - 1) {
      MPI_Send(&writeSignal, 1, MPI_DOUBLE, p + 1, tag, MPI_COMM_WORLD);
    }
  }
  /* That's it */
  MPI_Finalize();
  exit(0);
}