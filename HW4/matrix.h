#ifndef MATRIX_H
#define MATRIX_H

struct matrix {
  double **m;
  int rows, cols;
  int lastcol;
} matrix;


//Matrix ops
void make_translate(struct matrix *m, float tx, float ty, float tz);
void make_scale(struct matrix *m, float sx, float sy, float sz);
void make_rotate(struct matrix *m, int axis, float rot);

//Basic matrix manipulation routines
struct matrix *new_matrix(int rows, int cols);
void free_matrix(struct matrix *m);
void grow_matrix(struct matrix *m, int newcols);
void copy_matrix(struct matrix *a, struct matrix *b);
void print_matrix(struct matrix *m);
void ident(struct matrix *m);
void scalar_mult(double x, struct matrix *m);
void matrix_mult(struct matrix *a, struct matrix *b);

#endif
