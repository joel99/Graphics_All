#include <stdio.h>
#include <stdlib.h>

#include "ml6.h"
#include "display.h"
#include "draw.h"
#include "matrix.h"

#include <math.h>
#define M_PI 3.14159265358979323846

/*======== void add_circle() ==========
  Inputs:   struct matrix * points
            double cx
	    double cy
	    double r
	    double step  `
  Returns: 

  Adds the circle at (cx, cy) with radius r to points
  ====================*/
void add_circle( struct matrix * points, 
		 double cx, double cy, double cz,
		 double r, double step ) {

  double t, x1, y1, theta;
  double x2 = cx + r;
  double y2 = cy;

  while (t < 1.001){
    theta = 2 * M_PI * t;
    x1 = x2;
    y1 = y2;
    x2 = cx + r * cos(theta);
    y2 = cy + r * sin(theta);

    add_edge(points, x1, y1, cz, x2, y2, cz);
    t += step;
  }
  
}

/*======== void add_curve() ==========
Inputs:   struct matrix *points
         double x0
         double y0
         double x1
         double y1
         double x2
         double y2
         double x3
         double y3
         double step
         int type  
Returns: 

Adds the curve bounded by the 4 points passsed as parameters
of type specified in type (see matrix.h for curve type constants)
to the matrix points
====================*/
void add_curve( struct matrix *points, 
		double x0, double y0, 
		double x1, double y1, 
		double x2, double y2, 
		double x3, double y3, 
		double step, int type ) {

  if (type == 0) printf("Bezier\n");
  else printf("Hermite\n");
  struct matrix *curveX = generate_curve_coefs(x0, x1, x2, x3, type);
  struct matrix *curveY = generate_curve_coefs(y0, y1, y2, y3, type);
  printf("x\n");
  print_matrix(curveX);

  printf("y\n");
  print_matrix(curveY);


  
  double t = 0.0;
  double xa = 0.0;
  double ya = 0.0;
  double xb = x0;
  double yb = y0;
  
  struct matrix *matrixT = new_matrix(1,4);
  matrixT->lastcol = 4;
  while (t < 1.001){
    
    /*
    matrixT->m[0][0] = t * t * t;
    matrixT->m[0][1] = t * t;
    matrixT->m[0][2] = t;
    matrixT->m[0][3] = 1;
    printf("MatrixT\n");
    print_matrix(matrixT);
    */
    xa = xb;
    ya = yb;
  
    //xb = dot_product(curveX, matrixT);
    //yb = dot_product(curveY, matrixT);
    
    xb = (curveX->m[0][0] * t * t * t) + (curveX->m[1][0] * t * t) + (curveX->m[2][0] * t) + curveX->m[3][0];
    
    yb = (curveY->m[0][0] * t * t * t) + (curveY->m[1][0] * t * t) + (curveY->m[2][0] * t) + curveY->m[3][0];
    
    add_edge(points, xa, ya, 0, xb, yb, 0);
    t += step;
    
  }
  
  free(curveX);
  free(curveY);
 
  
}

/*======== void add_point() ==========
Inputs:   struct matrix * points
         int x
         int y
         int z 
Returns: 
adds point (x, y, z) to points and increment points.lastcol
if points is full, should call grow on points
====================*/
void add_point( struct matrix * points, double x, double y, double z) {

  if ( points->lastcol == points->cols )
    grow_matrix( points, points->lastcol + 100 );
  
  points->m[0][ points->lastcol ] = x;
  points->m[1][ points->lastcol ] = y;
  points->m[2][ points->lastcol ] = z;
  points->m[3][ points->lastcol ] = 1;
  points->lastcol++;
} //end add_point

/*======== void add_edge() ==========
Inputs:   struct matrix * points
          int x0, int y0, int z0, int x1, int y1, int z1
Returns: 
add the line connecting (x0, y0, z0) to (x1, y1, z1) to points
should use add_point
====================*/
void add_edge( struct matrix * points, 
	       double x0, double y0, double z0, 
	       double x1, double y1, double z1) {
  add_point( points, x0, y0, z0 );
  add_point( points, x1, y1, z1 );
}

/*======== void draw_lines() ==========
Inputs:   struct matrix * points
         screen s
         color c 
Returns: 
Go through points 2 at a time and call draw_line to add that line
to the screen
====================*/
void draw_lines( struct matrix * points, screen s, color c) {

 if ( points->lastcol < 2 ) {
   printf("Need at least 2 points to draw a line!\n");
   return;
 }
 
 int point;
 for (point=0; point < points->lastcol-1; point+=2)
   draw_line( points->m[0][point],
	      points->m[1][point],
	      points->m[0][point+1],
	      points->m[1][point+1],
	      s, c);	       
}// end draw_lines









void draw_line(int x0, int y0, int x1, int y1, screen s, color c) {
  
  int x, y, d, A, B;
  //swap points if going right -> left
  int xt, yt;
  if (x0 > x1) {
    xt = x0;
    yt = y0;
    x0 = x1;
    y0 = y1;
    x1 = xt;
    y1 = yt;
  }
  
  x = x0;
  y = y0;
  A = 2 * (y1 - y0);
  B = -2 * (x1 - x0);  

  //octants 1 and 8
  if ( abs(x1 - x0) >= abs(y1 - y0) ) {

    //octant 1    
    if ( A > 0 ) {
      
      d = A + B/2;      
      while ( x < x1 ) {
	plot( s, c, x, y );
	if ( d > 0 ) {
	  y+= 1;
	  d+= B;
	}
	x++;
	d+= A;
      } //end octant 1 while
      plot( s, c, x1, y1 );
    } //end octant 1

    //octant 8
    else {
      d = A - B/2;
      
      while ( x < x1 ) {
	//printf("(%d, %d)\n", x, y);
	plot( s, c, x, y );
	if ( d < 0 ) {
	  y-= 1;
	  d-= B;
	}
	x++;
	d+= A;
      } //end octant 8 while
      plot( s, c, x1, y1 );
    } //end octant 8
  }//end octants 1 and 8

  //octants 2 and 7
  else {
    
    //octant 2    
    if ( A > 0 ) {
      d = A/2 + B;      

      while ( y < y1 ) {
	plot( s, c, x, y );
	if ( d < 0 ) {
	  x+= 1;
	  d+= A;
	}
	y++;
	d+= B;
      } //end octant 2 while
      plot( s, c, x1, y1 );
    } //end octant 2

    //octant 7
    else {
      d = A/2 - B;
      
      while ( y > y1 ) {
	plot( s, c, x, y );
	if ( d > 0 ) {
	  x+= 1;
	  d+= A;
	}
	y--;
	d-= B;
      } //end octant 7 while
      plot( s, c, x1, y1 );
    } //end octant 7   
  }//end octants 2 and 7  
} //end draw_line
