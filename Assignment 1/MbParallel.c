#include <stdio.h>
#include <math.h>
#include <mpi.h>


void getColor(unsigned char *color,int it,int MaxIter)
{    
      if (it == MaxIter)
      { /*  interior of Mandelbrot set white */
        color[0] = 255;
        color[1] = 255;
        color[2] = 255;
      }
        else
      { /* exterior of Mandelbrot set = color gradient taken from reddit  ( same as wikipedia example) */
        int i = it % 16;
        switch (i)
        {
        case 0:
          color[0] = 66;
          color[1] = 30;
          color[2] = 15;
          break;
        case 1:
          color[0] = 25;
          color[1] = 7;
          color[2] = 26;
          break;
        case 2:
          color[0] = 9;
          color[1] = 1;
          color[2] = 47;

          break;
        case 3:
          color[0] = 4;
          color[1] = 4;
          color[2] = 73;
          break;
        case 4:
          color[0] = 0;
          color[1] = 7;
          color[2] = 100;
          break;
        case 5:
          color[0] = 12;
          color[1] = 44;
          color[2] = 138;

          break;
        case 6:
          color[0] = 24;
          color[1] = 82;
          color[2] = 177;

          break;
        case 7:
          color[0] = 57;
          color[1] = 125;
          color[2] = 209;

          break;
        case 8:
          color[0] = 134;
          color[1] = 181;
          color[2] = 229;

          break;
        case 9:
          color[0] = 211;
          color[1] = 236;
          color[2] = 248;

          break;
        case 10:
          color[0] = 241;
          color[1] = 233;
          color[2] = 191;

          break;
        case 11:
          color[0] = 248;
          color[1] = 201;
          color[2] = 95;

          break;
        case 12:
          color[0] = 248;
          color[1] = 201;
          color[2] = 95;

          break;
        case 13:
          color[0] = 204;
          color[1] = 128;
          color[2] = 0;

          break;
        case 14:
          color[0] = 153;
          color[1] = 87;
          color[2] = 0;

          break;
        case 15:
          color[0] = 106;
          color[1] = 52;
          color[2] = 3;

          break;
        default:
          color[0] = 0;
          color[1] = 0;
          color[2] = 0;
        }
      };
     
}
int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);

    int iX, iY;
    int xRes = 1024;
    int yRes = 1024;
    double Cx, Cy;
    double xMin = -2;
    double xMax = 2;
    double yMin = -2.0;
    double yMax = 2.0;
    double PixelWidth = (xMax - xMin) / xRes;
    double PixelHeight = (yMax - yMin) / yRes;

    double Zx, Zy;
    double Zx2, Zy2;

    int it;
    int MaxIter = 80;

    double Radius = 2;
    double R2 = Radius * Radius;

    int P = 8;
    int Q = 8;
    
    // size of x and y strip for each processor:
    int wp = xRes/P;     // wp must be divisible by P. This is the number of pixels each processor is responsible for (in width)
    int hp = yRes/Q;    // hp must be divisible by Q. This is the number of pixels each processor is responsible for (in height)

    // if(xRes % P != 0 || yRes % Q != 0) then
    // dimensionality error! P must divide xRes and Q must divide yRes
    
    int rank, size;

    int xOff = rank*wp; // (rank is the rank of processor p) (rank 0 starts to the left) this is offset to tell processor what x-coordinate to compute
    int yOff = rank*hp; //(rank 0 starts in bottom) same as above but y-coord


    int[] localColors = new int[wp*hp]; //array to fill with cols of all pixels in alloted grid
    int colorsSize = sizeof(localColors) / sizeof(localColors[0]); //Length of localColors array

    MPI_Comm_rank(MPI_COMM_WORLD, &rank); //Get processor rank
    MPI_Comm_size(MPI_COMM_WORLD, &size); //Get no. of processors

    for (iY = yOff; iY < (yOff + hp); iY++)
        {
            Cy = yMin + iY * PixelHeight;
            if (fabs(Cy) < PixelHeight / 2)
                Cy = 0.0;
            for (iX = xOff; iX < (xOff + wp); iX++)
            {
                Cx = xMin + iX * PixelWidth;
                Zx = 0.0;
                Zy = 0.0;
                Zx2 = Zx * Zx;
                Zy2 = Zy * Zy;
                /* */
                for (it = 0; it < MaxIter && ((Zx2 + Zy2) < R2); it++)
                {
                    Zy = 2 * Zx * Zy + Cy;
                    Zx = Zx2 - Zy2 + Cx;
                    Zx2 = Zx * Zx;
                    Zy2 = Zy * Zy;
                };

                getColor(color,it,MaxIter);

                colors[iY-yOff][0] = color[0];
                colors[iY-yOff][1] = color[1];
                colors[iY-yOff][2] = color[2];
            }
        }
    
    if(rank == 0){                  // Master processor
            
        FILE *fp;
        char *filename = "mandelbrot.ppm";
        static unsigned char color[3];
        
        for(p=1; p<P; p++){
            for(q=1; q<Q; q++){
                //MPI_Recv(); // receive information from all other processors
            }
        }
   
        fp = fopen(filename, "wb");
        //fwrite(color, 1, 3, fp); //write all colors to file
        fclose(fp);

    } else                      // Slave processor
    {
        //send color and coords to processor rank 0
        //MPI_send(0,localColors);
    }  
    

    MPI_Finalize();
    return 0;
    }
}
