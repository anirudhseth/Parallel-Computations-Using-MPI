#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

void printArray(double arr[], int size) {

  for (int i = 0; i < size; i++) {
    printf("%f ", arr[i]);
  }
  printf("\n");
}
void swap(double * x, double * y) {
  double t;
  t = * x;
  * x = * y;
  * y = t;
}
void compare(double arr[], int i, int j, bool dir) {
  if (dir == (arr[i] > arr[j]))
    swap( & arr[i], & arr[j]);
}

void bitonicMerge(double arr[], int low, int count, bool dir) {
  if (count > 1) {
    int index = count / 2;
    for (int i = low; i < low + index; i++)
      compare(arr, i, i + index, dir);
    bitonicMerge(arr, low, index, dir);
    bitonicMerge(arr, low + index, index, dir);
  }
}

void bitonicSort(double arr[], int low, int count, bool direction) {
  if (count > 1) {
    int index = count / 2;
    bitonicSort(arr, low, index, true);
    bitonicSort(arr, low + index, index, false);
    bitonicMerge(arr, low, count, direction);
  }
}

int main(int argc, char ** argv) {
  double * arr;
  int N = 10;
  srandom(1);
  /*generate random array between 0 and 1*/
  arr = (double * ) malloc(N * sizeof(double));
  for (int i = 0; i < N; i++) {
    arr[i] = ((double) random()) / (RAND_MAX);
  }
  printf("Orignal Array: ");
  printArray(arr, N);
  bool direction = true; /* true for ascending , false for descending*/
  bitonicSort(arr, 0, N, direction);
  printf("Sorted Array: ");
  printArray(arr, N);
  return 0;
}