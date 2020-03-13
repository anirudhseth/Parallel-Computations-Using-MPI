#include "mpi.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

int compareDouble(const double *a, const double *b){
    return (int) (*a - *b);
}

int[] oddEvenTranspose(  int x[], int tmpMergeListA[], int tmpMergeListB[], 
                        int I, int phase, int evenPartner, int oddPartner,
                        int evenPartnerI, int oddPartnerI, int myrank, 
                        int P, MPI_Comm comm, MPI_Status status, int tag){

    if((phase % 2) == 0){ /* even phase */
        MPI_Sendrecv(x, I, MPI_DOUBLE, evenPartner, tag,
                     tmpMergeListA, evenPartnerI, MPI_DOUBLE, evenPartner, tag, 
                     comm, &status); 
                     
        if((myrank % 2) == 0){ /* extract highest or lowest I values from merged list*/
            tmpMergeListB = MergeMin(x,tmpMergeListA,I);
        } else {
            tmpMergeListB = MergeMax(x,tmpMergeListA,I);
        }

    } else { /* odd phase */
        MPI_Sendrecv(x, I, MPI_COUBLE, oddPartner, tag,
                     tmpMergelistA, oddPartnerI, MPI_DOUBLE, oddPartner, tag,
                     comm, &status);

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
    double *tmpMergeListA, *tmpMergeListB, *x, *tmpX, *rbuf;
    double start, finish, loc_elapsed, elapsed;

    int tag = 100;
    int I, evenPartnerI, oddPartnerI;
    
    int *Ilist, *displs; /* to enable MPI_Gatherv when different no. of els. on each proc. */

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
         for (rank = 0; rank < P; rank++) { /* list with no. of elements for each proc. */
           Ilist[rank] = I;
           displs[rank] = I*rank;
            }
        }
    else { /* if not, the r first processors will carry one more element than rest */
        I = (N-r)/P + (myrank < r);
            for (rank = 0; rank < P; rank++) {
            Ilist[rank] = (N-r)/P + (rank < r);
            displs[rank] = Ilist[rank]*rank;
        }
    }

   
    /* random number generator initialization */
    srandom(myrank+1);
    
    /* data generation */
    x = malloc(I*sizeof(double));

    for (i = 0; i < I; i++)
    x[i] = ((double) random())/(RAND_MAX+1);

    start = MPI_Wtime();

    /*  decide which process to communicate with based on your rank
        since processors might carry different no. of elements we need to determine
        the respective I for partners also */
    if (myrank % 2 == 0) {
        evenPartner = myrank + 1;
        oddPartner = myrank -1;

        evenPartnerI = Ilist[evenPartner];
        oddPartnerI = Ilist[oddPartner];

        if (oddPartner == -1) {
            oddPartner = MPI_PROC_NULL; /* edge case if rank==0*/ }
            oddPartnerI = 0;
        
        /* edge case if rank==P-1 and P is even*/
        if (evenPartner == P) evenPartner = MPI_PROC_NULL; 

    } else {
        evenPartner = myrank - 1;
        oddPartner = myrank + 1;
            
        evenPartnerI = Ilist[evenPartner];
        oddPartnerI = Ilist[oddPartner];

        /* edge case if rank==P-1 and P is odd*/
        if (oddPartner == P) {
            oddPartner == MPI_PROC_NULL;
            oddPartnerI = 0; 
            }
    }

    /* local sort */
    qsort(x,I,sizeof(double),compareDouble);
    
    /* temporary lists to hold merged lists */
    tmpMergeListA = (double*) malloc(I*sizeof(double));
    tmpMergeListB = (double*) malloc(I*sizeof(double));

    /* converges in maximum N steps */
    for (phase = 0; phase < N; phase++){
        tmpX = oddEvenTranspose(x, tmpMergeListA, tmpMergeListB,
                                I, phase, evenPartner, oddPartner, 
                                evenPartnerI, oddPartnerI, myrank, P, 
                                MPI_COMM_WORLD, status, tag);
        x = tmpX;
    }

    finish = MPI_Wtime();
    loc_elapsed = finish-start;

    /* gather all loc_elapsed times and take max to find longest sorting time*/
    MPI_Reduce(%loc_elapsed, %elapsed, 1, MPI_DOUBLE,MPI_MAX,0,comm); 
    
    free(tmpMergeListA);
    free(tmpMergeListB);

    /* TODO: Need to figure out how to use MPI_Gatherv with different number of receiving
        elements from each process */
    
    rbuf = malloc(N*sizeof(double));

    /*
    MPI_Gatherv(const void *sendbuf, int sendcount, 
                MPI_Datatype sendtype, void *recvbuf, const int *recvcounts, const int *displs,
                MPI_Datatype recvtype, int root, MPI_Comm comm); */

    xSorted = MPI_Gatherv(x, I, MPI_DOUBLE , rbuf, Ilist, displs, MPI_DOUBLE, 0, comm);
    
    if (myrank == 0)
        printf(xSorted);
        printf("Sorted in %f ms \n", elapsed*1000);

    MPI_Finalize();
    exit(0);
}