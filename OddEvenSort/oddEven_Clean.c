#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

int compareDouble(const void *a, const void *b)
{
	if (*(double*) a > *(double*) b)
		return 1;
	else if (*(double*) a<*(double*) b)
		return -1;
	else
		return 0;
}


void mergeMax(double x[], double tmpMergeListA[], double tmpMergeListB[],
	int I)
{
	/*mergesort a and b */
	/*return n highest values */
	int a, b, c;
	a = b = c = I - 1;
	while (c >= 0)
	{
		if (x[a] >= tmpMergeListA[b])
		{
			tmpMergeListB[c--] = x[a--];
		}
		else
		{
			tmpMergeListB[c--] = tmpMergeListA[b--];

		}
	}

	memcpy(x, tmpMergeListB, I* sizeof(double));
}

void mergeMin(double x[], double tmpMergeListA[], double tmpMergeListB[],
	int I)
{
	/*mergesort a and b */
	/*return n lowest values */
	int a, b, c;
	a = b = c = 0;
	while (c < I)
	{
		if (x[a] <= tmpMergeListA[b])
		{
			tmpMergeListB[c++] = x[a++];
		}
		else
		{
			tmpMergeListB[c++] = tmpMergeListA[b++];
		}
	}

	memcpy(x, tmpMergeListB, I* sizeof(double));
}

void oddEvenTranspose(double x[], double tmpMergeListA[], double tmpMergeListB[],
	int I, int phase, int evenPartner, int oddPartner,
	int evenPartnerI, int oddPartnerI, int myrank,
	int P, MPI_Comm comm)
{
	MPI_Status status;
	if ((phase % 2) == 0)
	{ /*even phase */
		if (evenPartner >= 0)
		{
			MPI_Sendrecv(x, I, MPI_DOUBLE, evenPartner, 0,
				tmpMergeListA, I, MPI_DOUBLE, evenPartner, 0, comm, &status);
			if ((myrank % 2) == 0)
			{ /*extract highest or lowest I values from merged list*/
				mergeMin(x, tmpMergeListA, tmpMergeListB, I);
			}
			else
			{
				mergeMax(x, tmpMergeListA, tmpMergeListB, I);
			}
		}
	}
	else
	{ /*odd phase */
		if (oddPartner >= 0)
		{
			MPI_Sendrecv(x, I, MPI_DOUBLE, oddPartner, 0,
				tmpMergeListA, I, MPI_DOUBLE, oddPartner, 0, comm, &status);
			if ((myrank % 2) == 0)
			{
				mergeMax(x, tmpMergeListA, tmpMergeListB, I);
			}
			else
			{
				mergeMin(x, tmpMergeListA, tmpMergeListB, I);
			}
		}
	}
}´

int main(int argc, char **argv)
{
	int N, myrank, P;
	int phase;
	int evenPartner;
	int oddPartner;
	double *tmpMergeListA, *tmpMergeListB, *x, *tmpX, *rbuf;
	double start, finish, loc_elapsed, elapsed;
	int I, evenPartnerI, oddPartnerI;
	int *Ilist, *displs; /*to enable MPI_Gatherv when different no. of els. on each proc. */
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &P);
	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
	MPI_Status status;

	/*Find problem size N from command line */
	if (myrank == 0)
	{
		if (argc != 2 || atoi(argv[1]) < 1)
		{
			printf("Usage: mpirun -np<p> %s<n>\n", argv[0]);
			MPI_Abort(MPI_COMM_WORLD, 0);
		}
		else
		{
			N = atoi(argv[1]);
		}
	}
	/*local size. Modify if P does not divide N */
	MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);
	int r = N % P;

	if (r==0) { /*if P divides N */
	    I = N/P;
	    }

	else { /*if not, the r first processors will carry one more element than rest */
	    I = (N-r)/P + (myrank < r);
	}
	/*random number generator initialization */
	srandom(myrank + 1);
	/*data generation */
	x = (double*) malloc(I* sizeof(double));
	for (int i = 0; i < I; i++)
	{
		x[i] = ((double) random()) / (RAND_MAX);
	}
    if(N<20){
	printf("Process:%d has :", myrank);
	for (int i = 0; i < I; i++)
	{
		printf(" %f", x[i]);
	}

	printf("\n");}
	start = MPI_Wtime();
	if (myrank % 2 != 0)
	{
		evenPartner = myrank - 1;
		oddPartner = myrank + 1;
		if (oddPartner == P) oddPartner = -1;	
	}
	else
	{
		evenPartner = myrank + 1;
		if (evenPartner == P) evenPartner = -1;
		oddPartner = myrank - 1;
	}

	/*local sort */
	qsort(x, I, sizeof(double), compareDouble);

	/*temporary lists to hold merged lists */
	tmpMergeListA = (double*) malloc(I* sizeof(double));
	tmpMergeListB = (double*) malloc(I* sizeof(double));

	/*converges in maximum N steps */
	for (phase = 0; phase < P; phase++)
	{
		oddEvenTranspose(x, tmpMergeListA, tmpMergeListB,
			I, phase, evenPartner, oddPartner,
			evenPartnerI, oddPartnerI, myrank, P,
			MPI_COMM_WORLD);
		// x = tmpX;
	}

	finish = MPI_Wtime();
	free(tmpMergeListA);
	free(tmpMergeListB);
	double *Print = NULL;
	int i, n;

	if (myrank == 0)
	{
		Print = (double*) malloc(N* sizeof(double));
		MPI_Gather(x, I, MPI_DOUBLE, Print, I, MPI_DOUBLE, 0,
			MPI_COMM_WORLD);
      if(N<20){

		printf("******************************\n");
		printf("Final sorted List: ");
		for (i = 0; i < N; i++)
			printf("%f ", Print[i]);
		printf("\n");
		free(Print);}
		printf("N=%d P=%d Time= = %e seconds\n", N, P, finish - start);
      const char* filename = "RunningTime.txt";
      FILE * f;
      f = fopen(filename, "a");
      fprintf(f,"%d %d %e \n", N, P, finish - start);
		printf("******************************\n");
	}
	else
	{
		MPI_Gather(x, I, MPI_DOUBLE, Print, I, MPI_DOUBLE, 0,
			MPI_COMM_WORLD);
	}

	free(x);
	MPI_Finalize();
	return 0;
}