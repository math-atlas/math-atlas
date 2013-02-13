
include /home/msujon/ATLAS/ATLAS3.11.5/Linux_X8664_AVX_FMA/Make.inc

CC = gcc
CCFLAGS = 
LINKER = gcc
LFLAGS =
LB = ar
LBFLAGS = r
Atlas_lib = $(ATLASlib)


objects = inputf.o e_rem_pio2f.o k_rem_pio2f.o  s_floorf.o s_scalbnf.o \
	  s_signbitf.o s_copysignf.o s_fabsf.o

all: xinputf 

lib: inputf_lib 

e_rem_pio2f.o: e_rem_pio2f.c private.h
	$(CC) $(CCFLAGS) -o e_rem_pio2f.o -c e_rem_pio2f.c

k_rem_pio2f.o: k_rem_pio2f.c private.h
	$(CC) $(CCFLAGS) -o k_rem_pio2f.o -c k_rem_pio2f.c

s_floorf.o: s_floorf.c private.h
	$(CC) $(CCFLAGS) -o s_floorf.o -c s_floorf.c

s_scalbnf.o: s_scalbnf.c private.h
	$(CC) $(CCFLAGS) -o s_scalbnf.o -c s_scalbnf.c

s_signbitf.o: s_signbitf.c private.h
	$(CC) $(CCFLAGS) -o s_signbitf.o -c s_signbitf.c

s_copysignf.o: s_copysignf.c private.h
	$(CC) $(CCFLAGS) -o s_copysignf.o -c s_copysignf.c

s_fabsf.o: s_fabsf.c private.h
	$(CC) $(CCFLAGS) -o s_fabsf.o -c s_fabsf.c

inputf.o: inputf.c private.h
	$(CC) $(CCFLAGS) -o inputf.o -c inputf.c

xinputf: $(objects) main.c
	$(CC) $(CCFLAGS) -o main.o -c main.c
	$(LINKER) $(LFLAGS) -o $@ $(objects) main.o

inputf_lib: $(objects)
	$(LB) $(LBFLAGS) $(Atlas_lib) $(objects)

clean:
	rm -f $(objects) main.o xinputf 
