.PHONY: clean opencl

#vars
SO = $(shell uname -s)

ifeq ($(SO),Darwin)
  OPENCL_FLAGS=-framework OpenCL
else
  OPENCL_FLAGS=-lOpenCL
endif

GENERAL_OBJECTS=main.o opcl.o

#OPENCL
opencl: $(GENERAL_OBJECTS)
	g++ $(GENERAL_OBJECTS) -o rk $(OPENCL_FLAGS) -Wall -pedantic -Wextra

#general objects

main.o: main.c opcl.h
	gcc -c  main.c -o main.o -Wall -pedantic -Wextra
	
opcl.o: opcl.c opcl.h 
	gcc -c  opcl.c -o opcl.o -Wall -pedantic -Wextra

clean:
	rm -f *.o *.dat *~ *.p rk


