#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ml6.h"
#include "display.h"
#include "draw.h"
#include "matrix.h"

int main() {

  screen s;

  color c;
  c.red = 0;
  c.green = 0;
  c.blue = 255;
  
  struct matrix *edges;
  struct matrix *id;
  
  edges = new_matrix(4, 4);
  id = new_matrix(4, 4);

  printf("Identity Matrix:\n");
  ident(id);
  print_matrix(id);
  printf("\n");

  printf("Scalar Multiplication of Identity:\n");
  scalar_mult(.5, id);
  print_matrix(id);
  printf("\n");

  printf("Constructing initial edge matrix, drawn in blue\n");
  int i;
  for ( i = 0; i < 4; i++ )
    add_edge(edges, 50, 50, 0, 400, i * 100 + 50, 0);
  printf("Initial edge matrix\n");
  draw_lines(edges, s, c);

  
  c.blue = 0;
  c.red = 255;
  matrix_mult(id, edges);
  printf("Modified edge matrix\n");
  printf("Multiplied by .5 ID matrix, drawn in red\n");
  print_matrix(edges);
  printf("\n");

  

  draw_lines(edges, s, c);
  display(s);
  
  free_matrix( edges );
  free_matrix( id );
}  
