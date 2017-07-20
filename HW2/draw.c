#include <stdio.h>
#include <stdlib.h>

#include "ml6.h"
#include "display.h"
#include "draw.h"

//Insert your line algorithm here
void draw_line(int x0, int y0, int x1, int y1, screen s, color c) {
  
  int dy = y1 - y0;
  int dx = x1 - x0;

  //call drawline in here
  if (dx > 0){//1st half
    if (dy > 0){//1st quad
      draw_linePos(x0, y0, x1, y1, s, c);
    }
    else {//4th quad
      draw_lineNeg(x0, y0, x1, y1, s, c);    
    }
  }
  else {//2nd half
    if (dy > 0){//2nd quad
      draw_lineNeg(x1, y1, x0, y0, s, c);
    }
    else {//3rd quad
      draw_linePos(x1, y1, x0, y0, s, c);
    }
  }
  
}

void draw_linePos(int x0, int y0, int x1, int y1, screen s, color c) {

  int x = x0;
  int y = y0;
  //Ax + By + C = 0;
  //(y1 - y0)(x - x0) = (x1 - x0)(y-y0)  
  int a = y1 - y0;
  int b = x0 - x1;
  int d;
  int steep;
  
  if (abs(b) < abs(a)){
    d = a + 2 * b;
    steep = 1;
  }
  else {
    d = 2 * a + b;
    steep = 0;
  }

  a = 2 * a;
  
  b = 2 * b;
  
  while ((!steep && x < x1) || (steep && y < y1)){

    plot(s,c,x,y);

    if (!steep && d > 0){

      y++;
      
      d += b;

    }

    if (steep && d < 0){

      x++;

      d += a;
      
    }
    
    if (!steep){
      x++;
      d += a;
    }
    
    else {
      y++;
      d += b;
    }
    
  }
  
}

//QUAD VIII
void draw_lineNeg(int x0, int y0, int x1, int y1, screen s, color c) {

  int x = x0;
  int y = y0;
  //Ax + By + C = 0;
  //(y1 - y0)(x - x0) = (x1 - x0)(y-y0)  
  int a = y1 - y0;
  int b = x0 - x1;
  int d;
  int steep;
  
  if (abs(b) < abs(a)){
    d = a - 2 * b;
    steep = 1;    
  }
  else {
    d = 2 * a - b;
    steep = 0;
  }

  a = 2 * a;
  
  b = 2 * b;

  while ((!steep && x < x1) || (steep && y > y1)){

    plot(s, c, x, y);

      
    if (!steep && d < 0){

      y--;
      d -= b;

    }

    if (steep && d > 0){

      x++;
      d += a;
      
    }
    
    if (!steep){

      x++;
      d += a;

    }
    
    if (steep){

      y--;
      d -= b;

    }
    
  }
  
}
