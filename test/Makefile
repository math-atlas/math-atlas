SHELL = /bin/tcsh
CC = gcc
CCFLAGS = -g
IFKO = ../bin/ifko

all : irun

irun : xitst0 xitst1 xitst2 xitst3
	./xitst0
	./xitst1
	./xitst2
	./xitst3

xitst3 : mitst0.o itst3.o
	$(CC) $(CCFLAGS) -o $@ mitst0.o itst3.o
xitst2 : mitst2.o itst2.o
	$(CC) $(CCFLAGS) -o $@ mitst2.o itst2.o
xitst1 : mitst1.o itst1.o
	$(CC) $(CCFLAGS) -o $@ mitst1.o itst1.o
xitst0 : mitst0.o itst0.o
	$(CC) $(CCFLAGS) -o $@ mitst0.o itst0.o
xftst0 : mftst0.o ftst0.o
	$(CC) $(CCFLAGS) -o $@ mftst0.o ftst0.o
build_comp:
	cd ../bin ; $(MAKE) ifko

clean :
	rm -f x* *.o *.s
%.o : %.s
	$(CC) $(CCFLAGS) -c -o $@ $<
%.s : %.b 
	$(MAKE) build_comp
	$(IFKO) $< > $@
%.o : %.c
	$(CC) $(CCFLAGS) -c -o $@ $<
