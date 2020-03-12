#include "mpi.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

int compareDoubles(const double *a, const double *b){
    return (int) (*a - *b);
}

int[] oddEvenTranspose(  int x[], int tmpMergeListA[], int tmpMergeListB[], 
                        int I, int phase, int evenPartner, int oddPartner,
                        int evenPartnerI, int oddPartnerI, int myrank, 
                        int P, MPI_Comm comm, MPI_Status status, int tag){

    if((phase % 2) == 0){ /* even phase */
        MPI_Send(x, I, MPI_INT, evenPartner, tag, comm, &status);
        MPI_Recv(tmpMergeListA, evenPartnerI, MPI_INT,evenPartner, tag, comm, &status);

        if((myrank % 2) == 0){ /* extract highest or lowest I values from merged list*/
            tmpMergeListB = MergeMin(x,tmpMergeListA,I);
        } else {
            tmpMergeListB = MergeMax(x,tmpMergeListA,I);
        }

    } else { /* odd phase */
        MPI_Recv(tmpMergeListA, oddPartnerI, MPI_INT, oddPartner, tag, comm, &status);
        MPI_Send(x, I, MPI_INT, oddPartner, tag, comm, &status);

        if((myrank % 2) == 0){
            tmpMergeListB = mergeMax(x,tmpMergeListA,I);
        } else {
            tmpMergeListB = mergeMin(x,tmpMergeListA,I);
        }
    }

    return tmpMergeListB;
}

/* TODO: Need to implement these. Not sure how to mergesort lists */
int[] mergeMax(int a[], int b[], int n){
    /* mergesort a and b */
    /* return n highest values */
    }

int[] mergeMin(int a[], int b[], int n){
    /* mergesort a and b */
    /* return n lowest values */
}

int main(int argc, char **argv)
{
    int phase;
    int evenPartner;
    int oddPartner;
    int *tmpMergeListA, *tmpMergeListB, *x, *tmpX;
    
    int tag = 100;
    int I, evenPartnerI, oddPartnerI;

    MPI_Init(&argc, &argv);
    comm = MPI_COMM_WORLD;
    MPI_Comm_size(comm, &P);
    MPI_Comm_rank(comm, &myrank);
    MPI_Status status;

    /* Find problem size N from command line */
    if (argc < 2) error_exit(“No size N given”);
    int N = atoi(argv[1]);
    /* local size. Modify if P does not divide N */

    int r = P % N;
    
    if (r==0) { /* if P divides N */
        I = N/P;
        }
        else { /* if not, the r first processors will carry one more element than rest */
            I = (N-r)/P + (myrank < r);
        }

    /* random number generator initialization */
    srandom(myrank+1);
    /* data generation */
    for (i = 0; i < I; i++)
    x[i] = ((double) random())/(RAND_MAX+1);

    /*  decide which process to communicate with based on your rank
        since processors might carry different no. of elements we need to determine
        the respective I for partners also */
    if (myrank % 2 == 0) {
        evenPartner = myrank + 1;
        oddPartner = myrank -1;

        evenPartnerI = (N-r)/P + (evenPartner < r);
        oddPartnerI = (N-r)/P + (oddPartner < r);

        if (oddPartner == -1) {
            oddPartner = MPI_PROC_NULL; /* edge case if rank==0*/ }
            oddPartnerI = 0;
        
        /* edge case if rank==P-1 and P is even*/
        if (evenPartner == P) evenPartner = MPI_PROC_NULL; 

    } else {
        evenPartner = myrank - 1;
        evenPartnerI = (N-r)/P + (evenPartner < r);

        oddPartner = myrank + 1;
        oddPartnerI = (N-r)/P + (oddPartner < r);

        /* edge case if rank==P-1 and P is odd*/
        if (oddPartner == P) {
            oddPartner == MPI_PROC_NULL;
            oddPartnerI = 0; 
            }
        
    }

    /* local sort */
    qsort(x,I,sizeof(int),compareDoubles);
    
    /* temporary lists to hold merged lists */
    tmpMergeListA = (int*) malloc(I*sizeof(int));
    tmpMergeListB = (int*) malloc(I*sizeof(int));

    /* should converge in N steps */
    for (phase = 0; phase < N; phase++){
        tmpX = oddEvenTranspose(x, tmpMergeListA, tmpMergeListB,
                                I, phase, evenPartner, oddPartner, 
                                evenPartnerI, oddPartnerI, myrank, P, 
                                MPI_COMM_WORLD, status, tag)
        x = tmpX;
    }

    free(tmpMergeListA)
    free(tmpMergeListB)

    /* TODO: Need to figure out how to use MPI_Gatherv with different number of receiving
        elements from each process */
    /*
    if (myrank == 0) {
        b = MPI_Gatherv(a, Cint[I + 1 for _ in 0:r], MPI_INT , 0, comm);
    */

    MPI_Finalize();
    exit(0);
}