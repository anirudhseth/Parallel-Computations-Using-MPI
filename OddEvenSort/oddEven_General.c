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
	int I,int mI)
{
	/*mergesort a and b */
	/*return n highest values */
	
	int a, b, c;
	a = c = I - 1;
	b = sizeof(tmpMergeListA)/sizeof(double);
	
	while (c >= 0)
	{	
		if (b > -1){
			if (x[a] >= tmpMergeListA[b])
			{
			tmpMergeListB[c--] = x[a--];
			}
			else
			{
			tmpMergeListB[c--] = tmpMergeListA[b--];

			}
		} else {
			tmpMergeListB[c--] = x[a--];
		}
		
	}

	memcpy(x, tmpMergeListB, I* sizeof(double));
}

void mergeMin(double x[], double tmpMergeListA[], double tmpMergeListB[],
	int I,int mI)
{
	/*mergesort a and b */
	/*return n lowest values */
	int a, b, c;
	a = b = c = 0;
	int tmpMergeListAI = sizeof(tmpMergeListA)/sizeof(double);
	
	while (c < I)
	{
		if (b < tmpMergeListAI){

			
			if (x[a] <= tmpMergeListA[b])
			{
				tmpMergeListB[c++] = x[a++];
			}
			else
			{
				tmpMergeListB[c++] = tmpMergeListA[b++];
			}
		} else {
			tmpMergeListB[c++] = x[a++];
		}
	}

	memcpy(x, tmpMergeListB, I* sizeof(double));
}

void oddEvenTranspose(double x[], double tmpMergeListAOdd[], double tmpMergeListAEven, double tmpMergeListB[],
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
				tmpMergeListAEven, evenPartnerI, MPI_DOUBLE, evenPartner, 0, comm, &status);
				
			
			if ((myrank % 2) == 0)
			{ /*extract highest or lowest I values from merged list*/
				mergeMin(x, tmpMergeListAEven, tmpMergeListB, I,evenPartnerI);
			}
			else
			{
				mergeMax(x, tmpMergeListAEven, tmpMergeListB, I,evenPartnerI);
			}
		}
	}
	else
	{ /*odd phase */
		if (oddPartner >= 0)
		{
			MPI_Sendrecv(x, I, MPI_DOUBLE, oddPartner, 0,
				tmpMergeListAOdd, oddPartnerI, MPI_DOUBLE, oddPartner, 0, comm, &status);
			if ((myrank % 2) == 0)
			{
				mergeMax(x, tmpMergeListAOdd, tmpMergeListB, I,oddPartnerI);
			}
			else
			{
				mergeMin(x, tmpMergeListAOdd, tmpMergeListB, I,oddPartnerI);
			}
		}
	}
}

int main(int argc, char **argv)
{
	int N, myrank, P;
	int phase;
	int evenPartner;
	int oddPartner;
	double *tmpMergeListAOdd, *tmpMergeListAEven, *tmpMergeListB, *x, *tmpX, *rbuf;
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
	int R = N % P;

	if (R==0) { /*if P divides N */
	    I = N/P;
	    }

	else { /*if not, the r first processors will carry one more element than rest */
	    I = (N-R)/P + (myrank < R);
		// printf("Processor %d and I is %d\n",myrank,I);
		

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
	// 	printf("*************After Sorting*************");
	// 	printf("Process:%d has :", myrank);
	// for (int i = 0; i < I; i++)
	// {
	// 	printf(" %f", x[i]);
	// }
	
	// printf("\n");

	/*temporary lists to hold merged lists */

	
	
	tmpMergeListB = (double*) malloc(I* sizeof(double));
	evenPartnerI = (N-R)/P + (evenPartner < R);
	oddPartnerI = (N-R)/P + (oddPartnerI < R);
	//int s= oddPartnerI >evenPartnerI? oddPartnerI:evenPartnerI;
	tmpMergeListAOdd = (double*) malloc(oddPartnerI* sizeof(double));
	tmpMergeListAEven = (double*) malloc(evenPartnerI* sizeof(double));
	// printf("Process %d . Even Partner %d has I %d,Odd Partner %d has I %d\n",myrank,evenPartner,evenPartnerI,oddPartner,oddPartnerI);
	// printf("s:%d\n",s);
	// exit(1);
	/*converges in maximum N steps */
	for (phase = 0; phase < P; phase++)
	{

		// printf("******Phase:%d******\n",phase);
		// printf("Process %d . Even Partner %d has I %d,Odd Partner %d has I %d\n",myrank,evenPartner,evenPartnerI,oddPartner,oddPartnerI);
		
		oddEvenTranspose(x, tmpMergeListAOdd, tmpMergeListAEven, tmpMergeListB,I, phase, evenPartner, oddPartner,evenPartnerI, oddPartnerI, myrank, P,
			MPI_COMM_WORLD);
		
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
      if(N<25){

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