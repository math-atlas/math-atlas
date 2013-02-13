include /home/msujon/ATLAS/ATLAS3.11.5/Linux_X8664_AVX_FMA/Make.inc

CC = gcc
CCFLAGS = 
LINKER = gcc
LFLAGS=
LB = ar
LBFLAGS = r
Atlas_lib = $(ATLASlib)


objects = inputd.o e_rem_pio2.o k_rem_pio2.o  s_floor.o s_scalbn.o s_signbit.o \
	  s_copysign.o

all: xinput
lib: input_lib 

e_rem_pio2.o: e_rem_pio2.c private.h
	$(CC) $(CCFLAGS) -o e_rem_pio2.o -c e_rem_pio2.c

k_rem_pio2.o: k_rem_pio2.c private.h
	$(CC) $(CCFLAGS) -o k_rem_pio2.o -c k_rem_pio2.c

s_floor.o: s_floor.c private.h
	$(CC) $(CCFLAGS) -o s_floor.o -c s_floor.c

s_scalbn.o: s_scalbn.c private.h
	$(CC) $(CCFLAGS) -o s_scalbn.o -c s_scalbn.c

s_signbit.o: s_signbit.c private.h
	$(CC) $(CCFLAGS) -o s_signbit.o -c s_signbit.c

s_copysign.o: s_copysign.c private.h
	$(CC) $(CCFLAGS) -o s_copysign.o -c s_copysign.c

inputd.o: input.c private.h
	$(CC) $(CCFLAGS) -o inputd.o -c input.c

xinput: $(objects) main.o
	$(CC) $(CCFLAGS) -o main.o -c main.c
	$(LINKER) $(LFLAGS) -o $@ $(objects) main.o

input_lib: $(objects)
	$(LB) $(LBFLAGS) $(Atlas_lib) $(objects)

clean:
	rm -f $(objects) main.o xinput

