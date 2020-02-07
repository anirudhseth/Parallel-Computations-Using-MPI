#include <stdio.h>
#include <math.h>

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
int main()
{

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
  FILE *fp;
  char *filename = "mandelbrot.ppm";
  static unsigned char color[3];
  double Zx, Zy;
  double Zx2, Zy2;
  int it;
  int MaxIter = 80;
  double Radius = 2;
  double R2 = Radius * Radius;


  fp = fopen(filename, "wb");
  fprintf(fp, "P6\n %d\n %d\n %d\n", xRes, yRes, 255);

  for (iY = 0; iY < yRes; iY++)
  {

    Cy = yMin + iY * PixelHeight;
    if (fabs(Cy) < PixelHeight / 2)
      Cy = 0.0;
    for (iX = 0; iX < xRes; iX++)
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
      fwrite(color, 1, 3, fp);
    }
  }
  fclose(fp);
  return 0;
}