#include <stdio.h>
#include "opcl.h"

cl_platform_id _platform;
cl_context _context;
cl_device_id* _devices;
cl_command_queue _queue;
cl_kernel _kernel;
cl_program _program;
cl_event _event;
cl_mem _opencl_points, _opencl_n_points;
/*(cl_ulong start, finish;
double total = 0;*/
unsigned int _devices_found;
unsigned int _device_used = 0;

void /*RK_OpenCL::*/opencl_create_platform(unsigned int num_platforms){
  cl_uint num_platforms_found;
  
  clGetPlatformIDs( 0, NULL, &num_platforms_found);
  if ( clGetPlatformIDs( num_platforms, &_platform, &num_platforms_found ) != CL_SUCCESS ){
    printf("\nERROR: Failed to create plataform.\n");
    exit(-1);
  }
}

void /*RK_OpenCL::*/opencl_get_devices_id() {

  clGetDeviceIDs(_platform, CL_DEVICE_TYPE_ALL, 0, NULL, &_devices_found);
  _devices = (cl_device_id*) malloc(_devices_found*(sizeof(cl_device_id)));
  
  if(clGetDeviceIDs( _platform, CL_DEVICE_TYPE_ALL, _devices_found, _devices, NULL) != CL_SUCCESS){
    printf("\nERROR: Failed to get devices id's.\n");
    exit(-1);
  }
}

void /*RK_OpenCL::*/opencl_create_context(){
  if( (_context = clCreateContext( 0, 1, _devices, NULL, NULL, NULL )) == NULL ){
    printf("\nERROR: Failed to create context.\n");
    exit(-1);
  }
}

void /*RK_OpenCL::*/opencl_create_queue(){
  if((_queue = clCreateCommandQueue(_context, _devices[_device_used], CL_QUEUE_PROFILING_ENABLE, NULL)) == NULL ){
    printf("\nERROR: Failed to create queue.\n");
    exit(-1);
  }
}

char* /*RK_OpenCL::*/load_program_from_source(int *size) {
  char* program_string;
  FILE* prog;

  prog = fopen(/*"core/opencl/*/"rk_kernel.cl", "r");
  fseek(prog, 0, SEEK_END);
  *size = ftell(prog);
  fseek(prog, 0, SEEK_SET);

  program_string = (char*) malloc((*size+1)*sizeof(char));
  *size = fread(program_string, 1, *size, prog);
  fclose(prog);
  program_string[*size] = '\0';

  return program_string;
}

void /*RK_OpenCL::*/opencl_build_program(){
  int err;
  char *build_log;
  size_t ret_val_size;
                
  err = clBuildProgram(_program, 0, NULL, NULL, NULL, NULL);
  if ( err != CL_SUCCESS ) {
    printf("\nERROR: Failed to build program.\n");
    clGetProgramBuildInfo(_program, _devices[_device_used], CL_PROGRAM_BUILD_LOG, 0, NULL, &ret_val_size);

    build_log = (char*) malloc((ret_val_size+1)*sizeof(char));
    clGetProgramBuildInfo(_program, _devices[_device_used], CL_PROGRAM_BUILD_LOG, ret_val_size, build_log, NULL);
    build_log[ret_val_size] = '\0';
    printf("BUILD LOG: \n %s\n", build_log);
    exit(-1);
  }
}

/*void profile_event (cl_event* profiler) {
  cl_ulong start, finish;
    
  clWaitForEvents(1, &event);
  if (clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, (size_t)sizeof(cl_ulong), &start, NULL) != CL_SUCCESS) printf("Erro!\n");
  if (clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, (size_t)sizeof(cl_ulong), &finish, NULL) != CL_SUCCESS) printf("Erro!\n");
  total += (double)(finish-start);
}*/

void /*RK_OpenCL::*/opencl_create_program(){
  char* program_source;
  int size;
  size_t prog_size;
  cl_int err;

  program_source = load_program_from_source(&size);
  prog_size = (size_t)size;
  _program = clCreateProgramWithSource(_context, 1, (const char**)&program_source, &prog_size, &err);

  if ( err != CL_SUCCESS ){
    printf("\nERROR: Failed to create program.\n");
    exit(-1);
  }
  opencl_build_program();
}

void /*RK_OpenCL::*/opencl_create_kernel(char* kernel_name){

  _kernel = clCreateKernel(_program, kernel_name, NULL);
  if ( _kernel == NULL ){
    printf("\nERROR: Failed to create kernel %s.\n",kernel_name);
    exit(-1);
  }
}

void /*RK_OpenCL::*/prepare_kernel(vector *v0, unsigned int count_v0, float h, int n_x,int n_y,int n_z, vector_field field, unsigned int max_points){
  cl_mem _opencl_v0, _opencl_count_v0, _opencl_h, _opencl_n_x, _opencl_n_y, _opencl_n_z, _opencl_max_points, _opencl_field;
  
  _opencl_v0 = clCreateBuffer(_context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(vector)*(count_v0), v0, NULL);
  _opencl_count_v0 = clCreateBuffer(_context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int), &count_v0, NULL);
  _opencl_h = clCreateBuffer(_context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float), &h, NULL);
  _opencl_n_x = clCreateBuffer(_context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int), &n_x, NULL);
  _opencl_n_y = clCreateBuffer(_context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int), &n_y, NULL);
  _opencl_n_z = clCreateBuffer(_context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int), &n_z, NULL);  
  _opencl_field = clCreateBuffer(_context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(vector)*n_x*n_y*n_z, field, NULL);
  _opencl_points = clCreateBuffer(_context, CL_MEM_WRITE_ONLY , sizeof(vector)*count_v0*max_points, NULL, NULL);
  _opencl_n_points = clCreateBuffer(_context, CL_MEM_WRITE_ONLY , sizeof(unsigned int)*(count_v0), NULL,NULL);
  _opencl_max_points = clCreateBuffer(_context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(unsigned int), &max_points, NULL);

  clSetKernelArg(_kernel, 0, sizeof(cl_mem), (void *)&_opencl_v0);
  clSetKernelArg(_kernel, 1, sizeof(cl_mem), (void *)&_opencl_count_v0);
  clSetKernelArg(_kernel, 2, sizeof(cl_mem), (void *)&_opencl_h);
  clSetKernelArg(_kernel, 3, sizeof(cl_mem), (void *)&_opencl_n_x);
  clSetKernelArg(_kernel, 4, sizeof(cl_mem), (void *)&_opencl_n_y);
  clSetKernelArg(_kernel, 5, sizeof(cl_mem), (void *)&_opencl_n_z);
  clSetKernelArg(_kernel, 6, sizeof(cl_mem), (void *)&_opencl_field);
  clSetKernelArg(_kernel, 7, sizeof(cl_mem), (void *)&_opencl_points);
  clSetKernelArg(_kernel, 8, sizeof(cl_mem), (void *)&_opencl_n_points);
  clSetKernelArg(_kernel, 9, sizeof(cl_mem), (void *)&_opencl_max_points);

  clFinish(_queue);
}

void opencl_run_kernel(unsigned int count_v0, unsigned int max_points, unsigned int *n_points, vector *points){
  size_t work_dim[1];
  unsigned int i, j;

  work_dim[0] = count_v0;
  clEnqueueNDRangeKernel(_queue, _kernel, 1, NULL, work_dim, NULL, 0, NULL, &_event);
  /*profile_event(&event);*/
  clReleaseEvent(_event);
  clFinish(_queue);

  if( clEnqueueReadBuffer(_queue, _opencl_n_points, CL_TRUE, 0, sizeof(unsigned int)*count_v0, n_points, 0, NULL, &_event) == CL_INVALID_VALUE ){
    printf("\nERROR: Failed to read buffer \"n_points\".\n");
    exit(-1);
  }
  /*profile_event(&event);*/
  clReleaseEvent(_event);
  if( clEnqueueReadBuffer(_queue, _opencl_points, CL_TRUE, 0, sizeof(vector)*count_v0*max_points, points, 0, NULL, &_event) == CL_INVALID_VALUE ){
    printf("\nERROR: Failed to read buffer \"points\".\n");
    exit(-1);
  }
  /*profile_event(&event);*/
  clReleaseEvent(_event);

  for(i = 0; i < count_v0; i++)
    for(j = 0; j < n_points[i]; j++)
     printf("%f %f %f\n", points[(i+7*j)].x,points[(i+7*j)].y,points[(i+7*j)].z);
  
  /*printf("%lf\n", total*NANO);*/
}

void /*RK_OpenCL::*/opencl_init(char* kernel_name, vector *v0, int count_v0, double h, int n_x, int n_y, int n_z, vector_field field/*, runge_kutta::Fiber **fibers*/){
  unsigned int max_points, available, *n_points, i, j;
  vector *points;

  opencl_create_platform(2);
  opencl_get_devices_id();
  opencl_create_context();
  opencl_create_queue();
  opencl_create_program();
  opencl_create_kernel(kernel_name);

  n_points = (unsigned int*) malloc(count_v0*sizeof(unsigned int));
  clGetDeviceInfo(*_devices,CL_DEVICE_GLOBAL_MEM_SIZE,sizeof(available),&available,NULL);
  max_points = (available*0.9)/(sizeof(vector)*count_v0);
  points = (vector*) malloc(count_v0*max_points*sizeof(vector));
  
  prepare_kernel(v0, count_v0, h, n_x, n_y, n_z, field, max_points);
  opencl_run_kernel(count_v0, max_points, n_points,points);

  /*printf("Coping the result...");
  *fibers = (runge_kutta::Fiber *) malloc(count_v0*sizeof(runge_kutta::Fiber));
  for(i = 0; i < count_v0; i++){
    (*fibers)[i] = runge_kutta::Fiber(n_points[i]);
    for(j = 0; j < n_points[i]; j++){
      (*fibers)[i].setPoint(j, points[(i+count_v0*j)]);
    }
  }
  printf(" OK.\n");*/
  
  free(n_points);
  free(points);  
}


