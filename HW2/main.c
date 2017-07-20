#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "ml6.h"
#include "display.h"
#include "draw.h"

#define PI 3.14159265

int main() {

  screen s;
  color c;
 
  
  c.red = 0;
  c.green = MAX_COLOR;
  c.blue = 0;
  
  clear_screen(s);

  c.red = MAX_COLOR;
  c.green = 0;
  draw_line(240, 250, 260, 250, s, c);
  draw_line(250, 240, 250, 260, s, c);
  
  int i;
  int waves;
  int magnitude = 0;

  for (waves = 16; waves > 4; waves-= 2){

    c.red = ((int)(waves * PI * 3) + 50) % 256;
    c.blue = (waves * 10 + 100) % 256;
    c.green = (waves * waves * waves + 150) % 256;     
    magnitude = 25 + 14 * waves;
    
    for (i = (waves % 5); i < 360; i += 3 - (waves % 2)){
    
    
      int addWave = (int)(25 * sin(waves * ((double)i * PI / 180)));
      draw_line(250 + (int)((magnitude) * sin((double)i * PI / 180)), 250 + (int)((magnitude) * cos((double)i * PI / 180)), 250 + (int)((magnitude + addWave) * sin((double)i * PI / 180)), 250 + (int)((magnitude + addWave) * cos((double)i * PI / 180)), s, c);

    }
    
  }


  
  display(s);
  save_extension(s, "lines.png");
  
}  
