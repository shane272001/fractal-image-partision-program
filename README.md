# fractal-image-partision-program
The mandel.c program creates a mandelbrot set and saves a single frame that is changeable in the code to an image file created by the program.
The following are examples of inputs when running the program:
./mandel -x 0.286932 -y 0.014287 -s .0005 -m 50 -o mandel1.bmp 
./mandel -x 0.286932 -y 0.014287 -s .0005 -m 100 -o mandel2.bmp 
./mandel -x 0.286932 -y 0.014287 -s .0005 -m 500 -o mandel3.bmp 
./mandel -x 0.286932 -y 0.014287 -s .0005 -m 1000 -o mandel4.bmp 
./mandel -x 0.286932 -y 0.014287 -s .0005 -m 2000 -o mandel5.bmp 

The line to change to decide which frame to save is commented on in the code.
