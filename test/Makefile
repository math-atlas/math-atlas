SHELL = /bin/tcsh
CC = gcc
CCFLAGS = -g
IFKO = ../bin/ifko

all : irun frun

fbuild : xftst0 xftst1 xftst2
frun   : fbuild 
	./xftst0
	./xftst1
	./xftst2

ibuild : xitst0 xitst1 xitst2 xitst3
irun : ibuild
	./xitst0
	./xitst1
	./xitst2
	./xitst3


xitst4 : mitst4.o itst4.o
	$(CC) $(CCFLAGS) -o $@ mitst4.o itst4.o
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
xftst1 : mftst1.o ftst1.o
	$(CC) $(CCFLAGS) -o $@ mftst1.o ftst1.o
xftst2 : mftst2.o ftst2.o
	$(CC) $(CCFLAGS) -o $@ mftst2.o ftst2.o
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
