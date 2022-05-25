/*
 * Name: Shane Purdy
 * ID: 1001789955
 * 
 * Copyright (c) 2016 Trevor Bakker 
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
 */

#include "bitmap.h"

#include <pthread.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <string.h>


struct arguments
{
	struct bitmap *bm; // bitmap
	double xc;         // x center value
	double yc;         // y center value
	double s;          // scale value
	double iw;         // image width value
	double ih;         // image height value
	double m;          // max size value
	int n;             // number of threads
	int threadid;      // Thread id
};

int iteration_to_color( int i, int max );
int iterations_at_point( double x, double y, int max );
void * compute_image(void * arg);

void show_help()
{
	printf("Use: mandel [options]\n");
	printf("Where options are:\n");
	printf("-m <max>    The maximum number of iterations per point. (default=1000)\n");
	printf("-x <coord>  X coordinate of image center point. (default=0)\n");
	printf("-y <coord>  Y coordinate of image center point. (default=0)\n");
	printf("-s <scale>  Scale of the image in Mandlebrot coordinates. (default=4)\n");
	printf("-W <pixels> Width of the image in pixels. (default=500)\n");
	printf("-H <pixels> Height of the image in pixels. (default=500)\n");
	printf("-o <file>   Set output file. (default=mandel.bmp)\n");
	printf("-h          Show this help text.\n");
	printf("-n          Set the number of threads to run.\n");
	printf("\nSome examples are:\n");
	printf("mandel -x -0.5 -y -0.5 -s 0.2\n");
	printf("mandel -x -.38 -y -.665 -s .05 -m 100\n");
	printf("mandel -x 0.286932 -y 0.014287 -s .0005 -m 1000\n\n");
}

int main( int argc, char *argv[] )
{
	char c;

	// These are the default configuration values used
	// if no command line arguments are given.

	const char *outfile = "mandel.bmp";
	double xcenter = 0;
	double ycenter = 0;
	double scale = 4;
	int    image_width = 500;
	int    image_height = 500;
	int    max = 1000;
	int    n = 1;

	// For each command line argument given,
	// override the appropriate configuration value.

	while((c = getopt(argc, argv, "x:y:s:W:H:m:o:n:h")) != -1) 
	{
		switch(c) 
		{
			case 'x':
				xcenter = atof(optarg);
				break;
			case 'y':
				ycenter = atof(optarg);
				break;
			case 's':
				scale = atof(optarg);
				break;
			case 'W':
				image_width = atoi(optarg);
				break;
			case 'H':
				image_height = atoi(optarg);
				break;
			case 'm':
				max = atoi(optarg);
				break;
			case 'o':
				outfile = optarg;
				break;
			case 'n':
				n = atof(optarg);
				break;
			case 'h':
				show_help();
				exit(1);
				break;
		}
	}

	// Display the configuration of the image.
	printf("mandel: x=%lf y=%lf scale=%lf max=%d outfile=%s\n", xcenter, ycenter, scale,max, outfile);

	// Create a structure for the inputed set of arguments.
	struct arguments comp[n];
	pthread_t tid[n];
	// Compute the Mandelbrot image
	int i; // Variable for index
	for(i = 0; i < n; i++)
	{
		// Create a bitmap of the appropriate size and place it in the created struct.
		comp[i].bm = bitmap_create(image_width, image_height);
		// Fill the struct with the inputed (and set) data.
		comp[i].xc = xcenter;
		comp[i].yc = ycenter;
		comp[i].s = scale;
		comp[i].iw = image_width;
		comp[i].ih = image_height;
		comp[i].m = max;
		comp[i].n = n;
		comp[i].threadid = i;
		pthread_create(&tid[i], NULL, compute_image, (void *) &comp[i]); // Create thread
	}
	int index = 0;         // Set the index value for the next loop
	while(index < n)       // Loop to wait on threads
	{
		pthread_join(tid[index], NULL);
		index++;         // Increment the index value
	}
	// Save the image in the stated file. To choose which part of the image to save, change the value of n to which fractal you wish to save.
	for(index = 0; index < n; index++)
	{
		if(!bitmap_save(comp[index].bm, outfile)) 
		{
			fprintf(stderr, "mandel: couldn't write to %s: %s\n", outfile, strerror(errno));
			return 1;
		}
	}
	return 0;
}



/*
Compute an entire Mandelbrot image, writing each point to the given bitmap.
Scale the image to the range (xmin-xmax,ymin-ymax), limiting iterations to "max"
*/
void * compute_image(void * arg)
{
	int i;
	int j;
	struct arguments * comp = (struct arguments *) arg;
	
	// Obtain the height and width of the bitmap.
	int width = bitmap_width(comp->bm);
	int height = bitmap_height(comp->bm);
	
	// Calculating the mins and maxes, and placing them into values for this function's use.
	double xmin = comp->xc - comp->s;
	double xmax = comp->xc + comp->s;
	double ymin = comp->yc - comp->s;
	double ymax = comp->yc + comp->s;
	int max = comp->m;
	// Initializing the beginning and end values for the drawing part of the function.
	int begin = (comp->threadid*height)/comp->n;
	int end;
	if(comp->threadid == ((comp->n) - 1))
	{
		end = height;
	}
	else
	{
		end = (begin+height/comp->n)-1;
	}
	// For every pixel in the image...
	for(j = begin; j < end; j++)
	{
		for(i = 0 ; i < width; i++)
		{
			//printf("In width loop\n");
			// Determine the point in x,y space for that pixel.
			double x = xmin + i*(xmax - xmin)/width;
			double y = ymin + j*(ymax - ymin)/height;

			// Compute the iterations at that point.
			int iters = iterations_at_point(x, y, max);

			// Set the pixel in the bitmap.
			bitmap_set(comp[0].bm, i, j, iters);
		}
	}
	return 0;
}
/*
Return the number of iterations at point x, y
in the Mandelbrot space, up to a maximum of max.
*/

int iterations_at_point( double x, double y, int max )
{
	double x0 = x;
	double y0 = y;

	int iter = 0;

	while( (x*x + y*y <= 4) && iter < max ) 
	{

		double xt = x*x - y*y + x0;
		double yt = 2*x*y + y0;

		x = xt;
		y = yt;

		iter++;
	}

	return iteration_to_color(iter,max);
}

/*
Convert a iteration number to an RGBA color.
Here, we just scale to gray with a maximum of imax.
Modify this function to make more interesting colors.
*/

int iteration_to_color( int i, int max )
{
	int gray = 255*i/max;
	return MAKE_RGBA(gray,gray,gray,0);
}
