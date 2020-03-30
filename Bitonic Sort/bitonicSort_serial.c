#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*fucntion to print array */
void printArray(double **arr,int size)
{	
	
	printf("Array: ");
    for (int i = 0; i < size; i++)
	{
    	printf("%f ", (*arr)[i]);	
	}
		printf("\n");
}
int main(int argc, char **argv)
{   double *arr;
    int size=10;
	srandom(1);
	/*generate random array */
	arr = (double*) malloc(size* sizeof(double));
	for (int i = 0; i < size; i++)
	{
		arr[i] = ((double) random()) / (RAND_MAX);
	}
	printf("Number of elements: %d\n",size);
	printArray(&arr,size);
	
    	return 0;
}