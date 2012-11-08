#include "CL/opencl.h"
typedef struct vec{
  float x;
  float y;
  float z;
} vector;

typedef vector *vector_field;

void opencl_init(char* kernel_name, vector *v0, int count_v0, double h, int n_x, int n_y, int n_z, vector_field field);
