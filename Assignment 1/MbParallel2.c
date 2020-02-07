#include <stdio.h>
#include <math.h>
#include <mpi.h>
#include <stdlib.h>

int mandelBrot(double Cx, double Cy) {
  double Zx, Zy;
  double Zx2, Zy2;
  int it;
  int MaxIter = 80;
  Zx = 0.0;
  Zy = 0.0;
  Zx2 = Zx * Zx;
  Zy2 = Zy * Zy;

  for (it = 0; it < MaxIter && ((Zx2 + Zy2) < 4); it++) {
    Zy = 2 * Zx * Zy + Cy;
    Zx = Zx2 - Zy2 + Cx;
    Zx2 = Zx * Zx;
    Zy2 = Zy * Zy;
  };
  return it;
}

int main(int argc, char ** argv) {
  int P, rank;
  int xRes = 2048;
  int yRes = 2048;
  double Cx, Cy;
  double xMin = -2;
  double xMax = 2;
  double yMin = -2.0;
  double yMax = 2.0;
  double PixelWidth = (xMax - xMin) / xRes;
  double PixelHeight = (yMax - yMin) / yRes;
  FILE * fp;
  int * data, * data_start;

  MPI_Status status;
  MPI_Init( & argc, & argv);
  MPI_Comm_size(MPI_COMM_WORLD, & P);
  MPI_Comm_rank(MPI_COMM_WORLD, & rank);

  int wp = xRes / P;
  int start = rank * wp;
  int end = start + wp;

  data = (int * ) malloc(wp * yRes * sizeof(int));
  data_start = data;

  fprintf(fp, "P6\n %d\n %d\n %d\n", wp, yRes, 255);
  for (int iX = start; iX < end; iX++) {

    Cx = xMin + iX * PixelWidth;
    for (int iY = 0; iY < yRes; iY++) {
      Cy = yMin + iY * PixelHeight;
      if (fabs(Cy) < PixelHeight / 2)
        Cy = 0.0;

      int it = mandelBrot(Cx, Cy);
      * data++ = it;
    }
  }

  data = data_start;
  if (rank == 0) {
    fp = fopen("color.txt", "w");
    printf("Process %d completed.\n", rank);
    for (int i = 0; i < wp; i++) {
      for (int j = 0; j < yRes; j++) {
        fprintf(fp, "%hhu ", (unsigned char) data[j + i * yRes]);
      }
      fprintf(fp, "\n");
    }

    fclose(fp);

    for (int k = 1; k < P; k++) {
      MPI_Recv(data, wp * yRes, MPI_INTEGER, k, 0, MPI_COMM_WORLD, & status);
      printf("Process %d completed.\n", k);
      fp = fopen("color.txt", "a");
      for (int i = 0; i < wp; i++) {
        for (int j = 0; j < yRes; j++) {
          fprintf(fp, "%hhu ", (unsigned char) data[j + i * yRes]);
        }
        fprintf(fp, "\n");
      }

      fclose(fp);
    }
  } else {
    MPI_Send(data, wp * yRes, MPI_INTEGER, 0, 0, MPI_COMM_WORLD);
  }

  MPI_Finalize();
  return 0;
}