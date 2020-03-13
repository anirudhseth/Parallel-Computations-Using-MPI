
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

int compareDouble(const void *a, const void *b){
      if (*(double*)a > *(double*)b)
    return 1;
  else if (*(double*)a < *(double*)b)
    return -1;
  else
    return 0;  
}



/* TODO: Need to implement these. Not sure how to mergesort lists */
void mergeMax(double x[], double tmpMergeListA[], double tmpMergeListB[], 
        int I){
    /* mergesort a and b */
    /* return n highest values */
       int ai, bi, ci;
   
   ai = I-1;
   bi = I-1;
   ci = I-1;
   while (ci >= 0) {
      if (x[ai] >= tmpMergeListA[bi]) {
         tmpMergeListB[ci] = x[ai];
         ci--; ai--;
      } else {
         tmpMergeListB[ci] = tmpMergeListA[bi];
         ci--; bi--;
      }
   }

   memcpy(x, tmpMergeListB, I*sizeof(double));
    }

void mergeMin(double x[], double tmpMergeListA[], double tmpMergeListB[], 
        int I){
    /* mergesort a and b */
    /* return n lowest values */
       int ai, bi, ci;
   
   ai = 0;
   bi = 0;
   ci = 0;
   while (ci < I) {
      if (x[ai] <= tmpMergeListA[bi]) {
         tmpMergeListB[ci] = x[ai];
         ci++; ai++;
      } else {
         tmpMergeListB[ci] = tmpMergeListA[bi];
         ci++; bi++;
      }
   }

   memcpy(x, tmpMergeListB, I*sizeof(double));
}

void oddEvenTranspose(  double x[], double tmpMergeListA[], double tmpMergeListB[], 
                        int I, int phase, int evenPartner, int oddPartner,
                        int evenPartnerI, int oddPartnerI, int myrank, 
                        int P, MPI_Comm comm){
    MPI_Status status;               
    if((phase % 2) == 0){ /* even phase */
    if (evenPartner >= 0) {
        // // MPI_Sendrecv(x, I, MPI_DOUBLE, evenPartner, 0,
        //              tmpMergeListA,I, evenPartner, MPI_DOUBLE, evenPartner, 0, 
        //              comm, &status); 
MPI_Sendrecv(x, I, MPI_DOUBLE, evenPartner, 0, 
tmpMergeListA, I, MPI_DOUBLE, evenPartner, 0, comm,
&status);
        if((myrank % 2) == 0){ /* extract highest or lowest I values from merged list*/
            mergeMin(x,tmpMergeListA,tmpMergeListB,I);
        } else {
            mergeMax(x,tmpMergeListA,tmpMergeListB,I);
        }
    }
    } else { /* odd phase */
     if (oddPartner >= 0) {
        // MPI_Sendrecv(x, I, MPI_DOUBLE, oddPartner, 0,
        //              tmpMergeListA,I, oddPartner, MPI_DOUBLE, oddPartner, 0,
        //              comm, &status);
         MPI_Sendrecv(x, I, MPI_DOUBLE, oddPartner, 0, 
            tmpMergeListA, I, MPI_DOUBLE, oddPartner, 0, comm,
            &status);
        if((myrank % 2) == 0){
            mergeMax(x,tmpMergeListA,tmpMergeListB,I);
        } else {
            mergeMin(x,tmpMergeListA,tmpMergeListB,I);
        }
    }}

    
}

int main(int argc, char **argv)
{ 
    int N,myrank,P;
    int phase;
    int evenPartner;
    int oddPartner;
    double *tmpMergeListA, *tmpMergeListB, *x, *tmpX, *rbuf;
    double start, finish, loc_elapsed, elapsed;

    int tag = 100;
    int I, evenPartnerI, oddPartnerI;
    
    int *Ilist, *displs; /* to enable MPI_Gatherv when different no. of els. on each proc. */

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &P);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    MPI_Status status;
    
    /* Find problem size N from command line */
   if (myrank == 0) {
      if (argc != 2 || atoi(argv[1])<1) {
          printf("Usage: mpirun -np <p> %s <n>\n", argv[0]);
          MPI_Abort(MPI_COMM_WORLD,0);
      } else {
         N = atoi(argv[1]);
      }
   }  
    /* local size. Modify if P does not divide N */
    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);
    // int r = P % N;
    
    // if (r==0) { /* if P divides N */
    //     I = N/P;
    //      for (rank = 0; rank < P; rank++) { /* list with no. of elements for each proc. */
    //        Ilist[rank] = I;
    //        displs[rank] = I*rank;
    //         }
    //     }
    // else { /* if not, the r first processors will carry one more element than rest */
    //     I = (N-r)/P + (myrank < r);
    //         for (rank = 0; rank < P; rank++) {
    //         Ilist[rank] = (N-r)/P + (rank < r);
    //         displs[rank] = Ilist[rank]*rank;
    //     }
    // }
    I = N/P;
   
    /* random number generator initialization */
    srandom(myrank+1);
    
    /* data generation */
    x = (double*) malloc(I*sizeof(double));
    for (int i = 0; i < I; i++) {
      x[i] = ((double) random()) / (RAND_MAX);
    }
    
    printf("Process:%d has :",myrank);
        for (int i = 0; i < I; i++) {
      printf(" %f",x[i]);
    }
    printf("\n");
    start = MPI_Wtime();

    /*  decide which process to communicate with based on your rank
        since processors might carry different no. of elements we need to determine
        the respective I for partners also */
    // if (myrank % 2 == 0) {
    //     evenPartner = myrank + 1;
    //     oddPartner = myrank -1;
    //     evenPartnerI = Ilist[evenPartner];
    //     oddPartnerI = Ilist[oddPartner];
    //     if (oddPartner == -1) {
    //         oddPartner = MPI_PROC_NULL; /* edge case if rank==0*/ }
    //         oddPartnerI = 0;
    //     /* edge case if rank==P-1 and P is even*/
    //     if (evenPartner == P) evenPartner = MPI_PROC_NULL; 

    // } else {
    //     evenPartner = myrank - 1;
    //     oddPartner = myrank + 1;  
    //     evenPartnerI = Ilist[evenPartner];
    //     oddPartnerI = Ilist[oddPartner];

    //     /* edge case if rank==P-1 and P is odd*/
    //     if (oddPartner == P) {
    //         oddPartner == MPI_PROC_NULL;
    //         oddPartnerI = 0; 
    //         }
    // }
       if (myrank % 2 != 0) {
      evenPartner = myrank - 1;
      oddPartner = myrank + 1;
      if (oddPartner == P) oddPartner = -1;  // Idle during odd phase
   } else {
      evenPartner = myrank + 1;
      if (evenPartner == P) evenPartner = -1;  // Idle during even phase
      oddPartner = myrank-1;  
   }
    /* local sort */
    qsort(x,I,sizeof(double),compareDouble);
    
    /* temporary lists to hold merged lists */
    tmpMergeListA = (double*) malloc(I*sizeof(double));
    tmpMergeListB = (double*) malloc(I*sizeof(double));

    /* converges in maximum N steps */
    for (phase = 0; phase < P; phase++){
        oddEvenTranspose(x, tmpMergeListA, tmpMergeListB,
                                I, phase, evenPartner, oddPartner, 
                                evenPartnerI, oddPartnerI, myrank, P, 
                                MPI_COMM_WORLD);
        // x = tmpX;
    }

    finish = MPI_Wtime();
    // loc_elapsed = finish-start;

    /* gather all loc_elapsed times and take max to find longest sorting time*/
    // MPI_Reduce(%loc_elapsed, %elapsed, 1, MPI_DOUBLE,MPI_MAX,0,comm); 
    
    free(tmpMergeListA);
    free(tmpMergeListB);

    /* TODO: Need to figure out how to use MPI_Gatherv with different number of receiving
        elements from each process */
    
    
      

    // rbuf = malloc(N*sizeof(double));

    /*
    MPI_Gatherv(const void *sendbuf, int sendcount, 
                MPI_Datatype sendtype, void *recvbuf, const int *recvcounts, const int *displs,
                MPI_Datatype recvtype, int root, MPI_Comm comm); */

    // xSorted = MPI_Gatherv(x, I, MPI_DOUBLE , rbuf, Ilist, displs, MPI_DOUBLE, 0, comm);
    
    // if (myrank == 0)
    //     printf(xSorted);
    //     printf("Sorted in %f ms \n", elapsed*1000);
    
     double* Print = NULL;
   int i, n;

   if (myrank == 0) {
      
      Print = (double*) malloc(N*sizeof(double));
      MPI_Gather(x, I, MPI_DOUBLE, Print, I, MPI_DOUBLE, 0,
            MPI_COMM_WORLD);
      printf("******************************\n");
      printf("Final sorted List: ");
      for (i = 0; i < N; i++)
         printf("%f ", Print[i]);
      printf("\n");
      free(Print);
      printf("N=%d P=%d Time= = %e seconds\n",N,P,finish-start);
      printf("******************************");
   } else {
      MPI_Gather(x, I, MPI_DOUBLE, Print, I, MPI_DOUBLE, 0,
            MPI_COMM_WORLD);
   }


   free(x);
   MPI_Finalize();
   return 0;
}