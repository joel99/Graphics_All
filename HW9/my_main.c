/*========== my_main.c ==========

  This is the only file you need to modify in order
  to get a working mdl project (for now).

  my_main.c will serve as the interpreter for mdl.
  When an mdl script goes through a lexer and parser, 
  the resulting operations will be in the array op[].

  Your job is to go through each entry in op and perform
  the required action from the list below:

  push: push a new origin matrix onto the origin stack
  pop: remove the top matrix on the origin stack

  move/scale/rotate: create a transformation matrix 
                     based on the provided values, then 
		     multiply the current top of the
		     origins stack by it.

  box/sphere/torus: create a solid object based on the
                    provided values. Store that in a 
		    temporary matrix, multiply it by the
		    current top of the origins stack, then
		    call draw_polygons.

  line: create a line based on the provided values. Store 
        that in a temporary matrix, multiply it by the
	current top of the origins stack, then call draw_lines.

  save: call save_extension with the provided filename

  display: view the image live
  
  jdyrlandweaver
  =========================*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "parser.h"
#include "symtab.h"
#include "y.tab.h"

#include "matrix.h"
#include "ml6.h"
#include "display.h"
#include "draw.h"
#include "stack.h"

void my_main() {

  int i;
  struct matrix *tmp;
  struct stack *s;
  screen t;
  color g;

  g.red = 255;
  g.green = 0;
  g.blue = 0;
  
  double xvals[4];
  double yvals[4];
  double zvals[4];
  double r, r1;
  double theta;
  char axis;
  int type;
  double step = 0.1;

  s = new_stack();
  tmp = new_matrix(4, 1000);
  clear_screen( t );
  
  for (i=0;i<lastop;i++) {  
    switch (op[i].opcode) {

    case PUSH:
      push(s);
      break;

    case POP:
      pop(s);
      break;
      
    case MOVE:
      tmp = make_translate(
		 op[i].op.move.d[0],
		 op[i].op.move.d[1],
		 op[i].op.move.d[2]);
      matrix_mult(s->data[s->top], tmp);
      copy_matrix(tmp, s->data[s->top]);
      break;

    case SCALE:
      tmp = make_scale(
		 op[i].op.scale.d[0],
		 op[i].op.scale.d[1],
		 op[i].op.scale.d[2]);
      matrix_mult(s->data[s->top], tmp);      
      copy_matrix(tmp, s->data[s->top]);
      break;

    case ROTATE:
      theta = op[i].op.rotate.degrees * M_PI / 180.0;
      axis = op[i].op.rotate.axis;
      
      if ( axis == 0 ) 
	tmp = make_rotX( theta );
      else if ( axis == 1 )
	tmp = make_rotY( theta );
      else 
	tmp = make_rotZ( theta );
      matrix_mult(s->data[s->top], tmp);      
      copy_matrix(tmp, s->data[s->top]);
      break;

    case BOX:
      tmp = new_matrix(4, 0);
      add_box(tmp,
	      op[i].op.box.d0[0],
	      op[i].op.box.d0[1],
	      op[i].op.box.d0[2],
	      op[i].op.box.d1[0],
	      op[i].op.box.d1[1],
	      op[i].op.box.d1[2]);
      matrix_mult(s->data[s->top], tmp);
      draw_polygons(tmp, t, g);
      free_matrix(tmp);
      break;

    case SPHERE:
      tmp = new_matrix(4, 0);
      add_sphere(tmp,
		 op[i].op.sphere.d[0],
		 op[i].op.sphere.d[1],
		 op[i].op.sphere.d[2],
		 op[i].op.sphere.r, step);
      matrix_mult(s->data[s->top], tmp);
      draw_polygons(tmp, t, g);
      free_matrix(tmp);
      break;

    case TORUS:
      tmp = new_matrix(4, 0);
      add_torus(tmp,
		op[i].op.torus.d[0],
		op[i].op.torus.d[1],
		op[i].op.torus.d[2],
		op[i].op.torus.r0,
		op[i].op.torus.r1,
		step);
      matrix_mult(s->data[s->top], tmp);
      draw_polygons(tmp, t, g);
      free_matrix(tmp);
      break;

    case LINE:
      tmp = new_matrix(4, 0);
      add_edge(tmp,
	       op[i].op.line.p0[0],
	       op[i].op.line.p0[1],
	       op[i].op.line.p0[2],
	       op[i].op.line.p1[0],
	       op[i].op.line.p1[1],
	       op[i].op.line.p1[2]);      
      matrix_mult(s->data[s->top], tmp);
      draw_lines(tmp, t, g);
      free_matrix(tmp);
      break;

    case SAVE:
      save_extension(t, op[i].op.save.p->name);
      break;

    case DISPLAY:
      display(t);
      break;

    default:
      printf("Something's going on\n");
      break;
    }
  }

  //free_matrix(tmp);
  free_stack(s);
  
}
