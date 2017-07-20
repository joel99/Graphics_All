#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ml6.h"
#include "display.h"
#include "draw.h"
#include "matrix.h"
#include "parser.h"
#include <math.h>

#define LINE 1
#define IDENT 2
#define SCALE 3
#define MOVE 4
#define ROT 5
#define APPLY 6
#define DISP 7
#define SAVE 8


void parse_file( char * filename, struct matrix * transform, struct matrix * edges,  screen scrn){

  FILE *f;
  char line[256];
  char * s = line;
  clear_screen(scrn);

  if ( strcmp(filename, "stdin") == 0 ){ 
    f = stdin;
    printf("Enter commands\n");
  }
  else{
    f = fopen(filename, "r");
    printf("Reading from file\n");
  }
    
  int mode = 0;
  while(1){
    if ( mode != IDENT && mode != APPLY && mode != DISP ){
      fgets(line, 255, f);
      //printf("line length is %ul\n", strlen(line));
      line[strlen(line)-1]='\0'; //gets rid of newline
      printf(":%s:\n",line);
      s = line;
      if (strlen(line) == 0) return; //EOF
    }

    //mode key:
    //0 n/a
    //1 line
    //2 ident
    //3 scale
    //4 move
    //5 rotate
    //6 apply
    //7 display
    //8 save
    color c;
    struct matrix *tmp = new_matrix(4,4);
    if (mode == 0){
      if (!strcmp(s, "line")) mode = LINE;
      else if (!strcmp(s, "ident")) mode = IDENT;
      else if (!strcmp(s, "scale")) mode = SCALE;
      else if (!strcmp(s, "move")) mode = MOVE;
      else if (!strcmp(s, "rotate")) mode = ROT;
      else if (!strcmp(s, "apply")) mode = APPLY;
      else if (!strcmp(s, "display")) mode = DISP;
      else if (!strcmp(s, "save")) mode = SAVE;
      else if (!strcmp(s, "quit")) return;
      else {
	printf("Invalid command\n");
	return;
      }
    }
    else{
      //printf("MODE IS %d\n", mode);
      switch(mode){
      case LINE:;
	float x1, y1, z1, x2, y2, z2;
	sscanf(s, "%f %f %f %f %f %f", &x1, &y1, &z1, &x2, &y2, &z2);
	add_edge(edges, x1, y1, z1, x2, y2, z2);
	break;
      case IDENT:
	ident(transform);
	break;
      case SCALE:
      case MOVE:;
	float sx, sy, sz;
	sscanf(s, "%f %f %f", &sx, &sy, &sz);
	if (mode == SCALE) make_scale(tmp, sx, sy, sz);
	if (mode == MOVE) make_translate(tmp, sx, sy, sz);
	matrix_mult(tmp, transform);
	break;
      case ROT:;
	char * axis = strsep(&s, " ");
	float rot = atof(s);
	if (!strcmp(axis, "x")) make_rotate(tmp, 0, rot);
	if (!strcmp(axis, "y")) make_rotate(tmp, 1, rot);
	if (!strcmp(axis, "z")) make_rotate(tmp, 2, rot);
	matrix_mult(tmp, transform);
	break;
      case APPLY:

	matrix_mult(transform, edges);

	break;
      case DISP:
	clear_screen(scrn);
	c.red = 255;
	c.blue = 255;
	c.green = 255;
	draw_lines(edges, scrn, c);
	display(scrn);
	break;
      case SAVE:
	c.red = 255;
	c.blue = 255;
	c.green = 255;
	draw_lines(edges, scrn, c);
	save_extension(scrn, s);
	break;
      }
      free(tmp);
      mode = 0;
      line[0] = 0; //reset
      //printf("Mode reset\n");
    }
    
  }
  
}
