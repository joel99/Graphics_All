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

/*======== void first_pass() ==========
  Inputs:   
  Returns: 

  Checks the op array for any animation commands
  (frames, basename, vary)
  
  Should set num_frames and basename if the frames 
  or basename commands are present

  If vary is found, but frames is not, the entire
  program should exit.

  If frames is found, but basename is not, set name
  to some default value, and print out a message
  with the name being used.

  jdyrlandweaver
  ====================*/
void first_pass() {
  //in order to use name and num_frames
  //they must be extern variable

  extern int num_frames;
  extern char name[128]; 
  
  int frameBool = 0;
  strcpy(name, "img");//default basename
  int varyBool = 0;
  int i;
  
  for (i=0;i<lastop;i++) {
    
    switch (op[i].opcode)
      {
      case FRAMES:
	num_frames = op[i].op.frames.num_frames;
	frameBool = 1;
	break;
      case BASENAME:
	strcpy(name, op[i].op.basename.p->name);
	break;
      case VARY:
	varyBool = 1;
	break;
      }
  }
  
  if (varyBool && !frameBool){
    exit(0);
  }

  return;
}

/*======== struct vary_node ** second_pass() ==========
  Inputs:   
  Returns: An array of vary_node linked lists

  In order to set the knobs for animation, we need to keep
  a separate value for each knob for each frame. We can do
  this by using an array of linked lists. Each array index
  will correspond to a frame (eg. knobs[0] would be the first
  frame, knobs[2] would be the 3rd frame and so on).

  Each index should contain a linked list of vary_nodes, each
  node contains a knob name, a value, and a pointer to the
  next node.

  Go through the opcode array, and when you find vary, go 
  from knobs[0] to knobs[frames-1] and add (or modify) the
  vary_node corresponding to the given knob with the
  appropriate value. 

  jdyrlandweaver
  ====================*/
struct vary_node** second_pass() {

  extern int num_frames;

  int i, f;
  struct vary_node** allFrameKnobs = (struct vary_node**)malloc(sizeof(struct vary_node) * num_frames);
  
  for (f = 0; f < num_frames; f++){ 
        
    struct vary_node * top = NULL;  

    for (i=0;i<lastop;i++) {
            
      switch (op[i].opcode)
	{
	case VARY: //oh god, calculate value of knob for current frame
	  if (f >= op[i].op.vary.start_frame && f <= op[i].op.vary.end_frame){ 
	    float valDenom = op[i].op.vary.end_val - op[i].op.vary.start_val;
	    float frameDenom = op[i].op.vary.end_frame - op[i].op.vary.start_frame;
	    float frameNum = f - op[i].op.vary.start_frame;
	    float valNum = frameNum / frameDenom * valDenom; //delta val

	    struct vary_node * node = (struct vary_node * ) malloc(sizeof(struct vary_node));
	    strcpy(node->name, op[i].op.vary.p->name);
	    node->value = op[i].op.vary.start_val + valNum;
	    node->next = top;
	    top = node;

	  }
	  break;
	}
    }

    allFrameKnobs[f] = top;
  }

  return allFrameKnobs;
}


/*======== void print_knobs() ==========
  Inputs:   
  Returns: 

  Goes through symtab and display all the knobs and their
  currnt values

  jdyrlandweaver
  ====================*/
void print_knobs() {
  
  int i;

  printf( "ID\tNAME\t\tTYPE\t\tVALUE\n" );
  for ( i=0; i < lastsym; i++ ) {

    if ( symtab[i].type == SYM_VALUE ) {
      printf( "%d\t%s\t\t", i, symtab[i].name );

      printf( "SYM_VALUE\t");
      printf( "%6.2f\n", symtab[i].s.value);
    }
  }
}


/*======== void my_main() ==========
  Inputs: 
  Returns: 

  This is the main engine of the interpreter, it should
  handle most of the commadns in mdl.

  If frames is not present in the source (and therefore 
  num_frames is 1, then process_knobs should be called.

  If frames is present, the enitre op array must be
  applied frames time. At the end of each frame iteration
  save the current screen to a file named the
  provided basename plus a numeric string such that the
  files will be listed in order, then clear the screen and
  reset any other data structures that need it.

  Important note: you cannot just name your files in 
  regular sequence, like pic0, pic1, pic2, pic3... if that
  is done, then pic1, pic10, pic11... will come before pic2
  and so on. In order to keep things clear, add leading 0s
  to the numeric portion of the name. If you use sprintf, 
  you can use "%0xd" for this purpose. It will add at most
  x 0s in front of a number, if needed, so if used correctly,
  and x = 4, you would get numbers like 0001, 0002, 0011,
  0487

  jdyrlandweaver
  ====================*/
void my_main() {

  int i;
  struct matrix *tmp;
  struct stack *systems;
  screen t;
  color g;
  double step = 0.1;
  double theta;
  float knobVal;
  systems = new_stack();
  tmp = new_matrix(4, 1000);
  clear_screen( t );
  g.red = 0;
  g.green = 0;
  g.blue = 0;
  
  //first pass
  first_pass();

  //second pass
  struct vary_node** knobValues =  second_pass();

  //third pass
  //replace sym table with knob values

  extern int num_frames;

  int f;
  if (num_frames > 1){    
    for (f = 0; f < num_frames; f++){
      printf("We're on frame %d\n", f);

      struct vary_node * top = knobValues[f];
      while ( top != NULL ){
	set_value(lookup_symbol(top->name), top->value);
	printf("Knob %s set to %f\n", lookup_symbol(top->name), top->value );
	top = top->next;
      }     

      for (i=0;i<lastop;i++) {

	printf("%d: ",i);

	switch (op[i].opcode)
	  {
	  case SET:
	    set_value(op[i].op.set.p->name, op[i].op.set.p->s.value);
	    break;
	  case SETKNOBS:;
	    struct vary_node * top = knobValues[f];
	    int j;
	    for (j = 0; j < lastsym; j++){
	      if (symtab[j].type == SYM_VALUE)
		symtab[j].s.value = op[i].op.setknobs.value;
	    }
	    break;	  
	  case SPHERE:
	    printf("Sphere: %6.2f %6.2f %6.2f r=%6.2f",
		   op[i].op.sphere.d[0],op[i].op.sphere.d[1],
		   op[i].op.sphere.d[2],
		   op[i].op.sphere.r);
	    if (op[i].op.sphere.constants != NULL)
	      {
		printf("\tconstants: %s",op[i].op.sphere.constants->name);
	      }
	    if (op[i].op.sphere.cs != NULL)
	      {
		printf("\tcs: %s",op[i].op.sphere.cs->name);
	      }
	    add_sphere(tmp, op[i].op.sphere.d[0],
		       op[i].op.sphere.d[1],
		       op[i].op.sphere.d[2],
		       op[i].op.sphere.r, step);
	    matrix_mult( peek(systems), tmp );
	    draw_polygons(tmp, t, g);
	    tmp->lastcol = 0;
	    break;
	  case TORUS:
	    printf("Torus: %6.2f %6.2f %6.2f r0=%6.2f r1=%6.2f",
		   op[i].op.torus.d[0],op[i].op.torus.d[1],
		   op[i].op.torus.d[2],
		   op[i].op.torus.r0,op[i].op.torus.r1);
	    if (op[i].op.torus.constants != NULL)
	      {
		printf("\tconstants: %s",op[i].op.torus.constants->name);
	      }
	    if (op[i].op.torus.cs != NULL)
	      {
		printf("\tcs: %s",op[i].op.torus.cs->name);
	      }
	    add_torus(tmp,
		      op[i].op.torus.d[0],
		      op[i].op.torus.d[1],
		      op[i].op.torus.d[2],
		      op[i].op.torus.r0,op[i].op.torus.r1, step);
	    matrix_mult( peek(systems), tmp );
	    draw_polygons(tmp, t, g);
	    tmp->lastcol = 0;	  
	    break;
	  case BOX:
	    printf("Box: d0: %6.2f %6.2f %6.2f d1: %6.2f %6.2f %6.2f",
		   op[i].op.box.d0[0],op[i].op.box.d0[1],
		   op[i].op.box.d0[2],
		   op[i].op.box.d1[0],op[i].op.box.d1[1],
		   op[i].op.box.d1[2]);
	    if (op[i].op.box.constants != NULL)
	      {
		printf("\tconstants: %s",op[i].op.box.constants->name);
	      }
	    if (op[i].op.box.cs != NULL)
	      {
		printf("\tcs: %s",op[i].op.box.cs->name);
	      }
	    add_box(tmp,
		    op[i].op.box.d0[0],op[i].op.box.d0[1],
		    op[i].op.box.d0[2],
		    op[i].op.box.d1[0],op[i].op.box.d1[1],
		    op[i].op.box.d1[2]);
	    matrix_mult( peek(systems), tmp );
	    draw_polygons(tmp, t, g);
	    tmp->lastcol = 0;
	    break;
	  case LINE:
	    printf("Line: from: %6.2f %6.2f %6.2f to: %6.2f %6.2f %6.2f",
		   op[i].op.line.p0[0],op[i].op.line.p0[1],
		   op[i].op.line.p0[1],
		   op[i].op.line.p1[0],op[i].op.line.p1[1],
		   op[i].op.line.p1[1]);
	    if (op[i].op.line.constants != NULL)
	      {
		printf("\n\tConstants: %s",op[i].op.line.constants->name);
	      }
	    if (op[i].op.line.cs0 != NULL)
	      {
		printf("\n\tCS0: %s",op[i].op.line.cs0->name);
	      }
	    if (op[i].op.line.cs1 != NULL)
	      {
		printf("\n\tCS1: %s",op[i].op.line.cs1->name);
	      }
	    break;
	    
	  case MOVE:
	    knobVal = 1;
	    printf("Move: %6.2f %6.2f %6.2f",
		   op[i].op.move.d[0],op[i].op.move.d[1],
		   op[i].op.move.d[2]);
	    if (op[i].op.move.p != NULL)
	      {
		printf("\tknob: %s",op[i].op.move.p->name);
	      }
	    if (op[i].op.move.p != NULL && lookup_symbol(op[i].op.move.p->name) != NULL){
	      knobVal = lookup_symbol(op[i].op.move.p->name)->s.value;
	    }   
	    tmp = make_translate( op[i].op.move.d[0] * knobVal,
				  op[i].op.move.d[1] * knobVal,
				  op[i].op.move.d[2] * knobVal);
	    
	    matrix_mult(peek(systems), tmp);
	    copy_matrix(tmp, peek(systems));
	    tmp->lastcol = 0;
	    break;
	    
	  case SCALE:
	    knobVal = 1;
	    printf("Scale: %6.2f %6.2f %6.2f",
		   op[i].op.scale.d[0],op[i].op.scale.d[1],
		   op[i].op.scale.d[2]);
	    if (op[i].op.scale.p != NULL)
	      {
		printf("\tknob: %s",op[i].op.scale.p->name);
	      }
	    
	    if (op[i].op.scale.p != NULL && lookup_symbol(op[i].op.scale.p->name) != NULL){
	      knobVal = lookup_symbol(op[i].op.scale.p->name)->s.value;
	    }
	    tmp = make_scale( op[i].op.scale.d[0] * knobVal, 
			      op[i].op.scale.d[1] * knobVal,
			      op[i].op.scale.d[2] * knobVal);	   
	    matrix_mult(peek(systems), tmp);
	    copy_matrix(tmp, peek(systems));
	    tmp->lastcol = 0;
	    break;
	  case ROTATE:
	    knobVal = 1;
	    printf("Rotate: axis: %6.2f degrees: %6.2f",
		   op[i].op.rotate.axis,
		   op[i].op.rotate.degrees);
	    if (op[i].op.rotate.p != NULL)
	      {
		printf("\tknob: %s",op[i].op.rotate.p->name);
	      }
	    
	    if (op[i].op.rotate.p != NULL && lookup_symbol(op[i].op.rotate.p->name) != NULL){
	      knobVal = lookup_symbol(op[i].op.rotate.p->name)->s.value;
	    }
	    theta =  op[i].op.rotate.degrees * (M_PI / 180) * knobVal;
	    if (op[i].op.rotate.axis == 0 )
	      tmp = make_rotX( theta );
	    else if (op[i].op.rotate.axis == 1 )
	      tmp = make_rotY( theta );
	    else
	      tmp = make_rotZ( theta );
	    
	    matrix_mult(peek(systems), tmp);
	    copy_matrix(tmp, peek(systems));
	    tmp->lastcol = 0;
	    break;
	  case PUSH:
	    printf("Push");
	    push(systems);
	    break;
	  case POP:
	    printf("Pop");
	    pop(systems);
	    break;
	  case SAVE:
	    printf("Save: %s",op[i].op.save.p->name);
	    save_extension(t, op[i].op.save.p->name);
	    break;
	  case DISPLAY:
	    printf("Display");
	    display(t);
	    break;
	  }
	printf("\n");
      }
      //end of frame, save image
      char dir_name[100];
      sprintf(dir_name, "anim/%s%03d", name, f); //assuming three digits 
      save_extension(t, dir_name);
      printf("Frame #%d saved as %s\n", f, dir_name);
      // reset
      tmp->lastcol = 0;
      systems = new_stack();
      clear_screen(t);
    }
  }

  else{ //old code
    for (i=0;i<lastop;i++) {
      
      printf("%d: ",i);
      switch (op[i].opcode)
	{
	case SPHERE:
	  printf("Sphere: %6.2f %6.2f %6.2f r=%6.2f",
		 op[i].op.sphere.d[0],op[i].op.sphere.d[1],
		 op[i].op.sphere.d[2],
		 op[i].op.sphere.r);
	  if (op[i].op.sphere.constants != NULL)
	    {
	      printf("\tconstants: %s",op[i].op.sphere.constants->name);
	    }
	  if (op[i].op.sphere.cs != NULL)
	    {
	      printf("\tcs: %s",op[i].op.sphere.cs->name);
	    }
	  add_sphere(tmp, op[i].op.sphere.d[0],
		     op[i].op.sphere.d[1],
		     op[i].op.sphere.d[2],
		     op[i].op.sphere.r, step);
	  matrix_mult( peek(systems), tmp );
	  draw_polygons(tmp, t, g);
	  tmp->lastcol = 0;
	  break;
	case TORUS:
	  printf("Torus: %6.2f %6.2f %6.2f r0=%6.2f r1=%6.2f",
		 op[i].op.torus.d[0],op[i].op.torus.d[1],
		 op[i].op.torus.d[2],
		 op[i].op.torus.r0,op[i].op.torus.r1);
	  if (op[i].op.torus.constants != NULL)
	    {
	      printf("\tconstants: %s",op[i].op.torus.constants->name);
	    }
	  if (op[i].op.torus.cs != NULL)
	    {
	      printf("\tcs: %s",op[i].op.torus.cs->name);
	    }
	  add_torus(tmp,
		    op[i].op.torus.d[0],
		    op[i].op.torus.d[1],
		    op[i].op.torus.d[2],
		    op[i].op.torus.r0,op[i].op.torus.r1, step);
	  matrix_mult( peek(systems), tmp );
	  draw_polygons(tmp, t, g);
	  tmp->lastcol = 0;	  
	  break;
	case BOX:
	  printf("Box: d0: %6.2f %6.2f %6.2f d1: %6.2f %6.2f %6.2f",
		 op[i].op.box.d0[0],op[i].op.box.d0[1],
		 op[i].op.box.d0[2],
		 op[i].op.box.d1[0],op[i].op.box.d1[1],
		 op[i].op.box.d1[2]);
	  if (op[i].op.box.constants != NULL)
	    {
	      printf("\tconstants: %s",op[i].op.box.constants->name);
	    }
	  if (op[i].op.box.cs != NULL)
	    {
	      printf("\tcs: %s",op[i].op.box.cs->name);
	    }
	  add_box(tmp,
		  op[i].op.box.d0[0],op[i].op.box.d0[1],
		  op[i].op.box.d0[2],
		  op[i].op.box.d1[0],op[i].op.box.d1[1],
		  op[i].op.box.d1[2]);
	  matrix_mult( peek(systems), tmp );
	  draw_polygons(tmp, t, g);
	  tmp->lastcol = 0;
	  break;
	case LINE:
	  printf("Line: from: %6.2f %6.2f %6.2f to: %6.2f %6.2f %6.2f",
		 op[i].op.line.p0[0],op[i].op.line.p0[1],
		 op[i].op.line.p0[1],
		 op[i].op.line.p1[0],op[i].op.line.p1[1],
		 op[i].op.line.p1[1]);
	  if (op[i].op.line.constants != NULL)
	    {
	      printf("\n\tConstants: %s",op[i].op.line.constants->name);
	    }
	  if (op[i].op.line.cs0 != NULL)
	    {
	      printf("\n\tCS0: %s",op[i].op.line.cs0->name);
	    }
	  if (op[i].op.line.cs1 != NULL)
	    {
	      printf("\n\tCS1: %s",op[i].op.line.cs1->name);
	    }
	  break;
	case MOVE:
	  printf("Move: %6.2f %6.2f %6.2f",
		 op[i].op.move.d[0],op[i].op.move.d[1],
		 op[i].op.move.d[2]);
	  if (op[i].op.move.p != NULL)
	    {
	      printf("\tknob: %s",op[i].op.move.p->name);
	    }
	  tmp = make_translate( op[i].op.move.d[0],
				op[i].op.move.d[1],
				op[i].op.move.d[2]);
	  matrix_mult(peek(systems), tmp);
	  copy_matrix(tmp, peek(systems));	  
	  tmp->lastcol = 0;
	  break;
	case SCALE:
	  printf("Scale: %6.2f %6.2f %6.2f",
		 op[i].op.scale.d[0],op[i].op.scale.d[1],
		 op[i].op.scale.d[2]);
	  if (op[i].op.scale.p != NULL)
	    {
	      printf("\tknob: %s",op[i].op.scale.p->name);
	    }
	  tmp = make_scale( op[i].op.scale.d[0],
			    op[i].op.scale.d[1],
			    op[i].op.scale.d[2]);
	  matrix_mult(peek(systems), tmp);
	  copy_matrix(tmp, peek(systems));
	  tmp->lastcol = 0;
	  break;
	case ROTATE:
	  printf("Rotate: axis: %6.2f degrees: %6.2f",
		 op[i].op.rotate.axis,
		 op[i].op.rotate.degrees);
	  if (op[i].op.rotate.p != NULL)
	    {
	      printf("\tknob: %s",op[i].op.rotate.p->name);
	    }
	  theta =  op[i].op.rotate.degrees * (M_PI / 180);
	  if (op[i].op.rotate.axis == 0 )
	    tmp = make_rotX( theta );
	  else if (op[i].op.rotate.axis == 1 )
	    tmp = make_rotY( theta );
	  else
	    tmp = make_rotZ( theta );
	  
	  matrix_mult(peek(systems), tmp);
	  copy_matrix(tmp, peek(systems));
	  tmp->lastcol = 0;
	  break;
	case PUSH:
	  printf("Push");
	  push(systems);
	  break;
	case POP:
	  printf("Pop");
	  pop(systems);
	  break;
	case SAVE:
	  printf("Save: %s",op[i].op.save.p->name);
	  save_extension(t, op[i].op.save.p->name);
	  break;
	case DISPLAY:
	  printf("Display");
	  display(t);
	  break;
	}
      printf("\n");
    }
  }
  printf("Awesome\n");
}
