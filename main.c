#include <stdio.h>
#include "opcl.h"

int main() {

  unsigned i,j,k, n_x, n_y, n_z, count_v0;
  float h;
  vector_field field;
  vector *v0;
  FILE *fp;
  
  fp = fopen("../TCC/runge-kutta/rotationField", "r");
  
  /*field = (vector_field *) malloc(sizeof(vector_field *));*/
  
  fscanf(fp, "%u", &n_x);
  fscanf(fp, "%u", &n_y);
  fscanf(fp, "%u", &n_z);

  fscanf(fp, "%u", &count_v0);

  v0 = (vector *) malloc( (count_v0)*sizeof(vector) );
  for(i = 0; i < count_v0; i++){
    fscanf(fp, "%f", &(((v0)[i]).x));
    fscanf(fp, "%f", &(((v0)[i]).y));
    fscanf(fp, "%f", &(((v0)[i]).z));
  }
  
  fscanf(fp, "%f", &h);

  field = (vector_field) malloc( n_x*n_y*n_z*sizeof(vector) );

  for(k = 0; k < n_z; k++){
    for(i = 0; i < n_x; i++){
      for(j = 0; j < n_y; j++){
        fscanf(fp, "%f", &(((field)[(i + n_x*j + n_y*n_x*k)]).x));
        fscanf(fp, "%f", &(((field)[(i + n_x*j + n_y*n_x*k)]).y));
        fscanf(fp, "%f", &(((field)[(i + n_x*j + n_y*n_x*k)]).z));
      }
    }
  }
 
  opencl_init("rk4_kernel", v0, count_v0, h, n_x, n_y, n_z, field);
  return 0;
}
