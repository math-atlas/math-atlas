SHELL = /bin/tcsh
CC = gcc
CCFLAGS = -g
IFKO = ../bin/ifko

all : ibuild fbuild irun frun

fbuild : xftst0 xftst1 xftst2 xftst3 xftst4 xftst5 xftst6 xftst7
frun   : fbuild 
	./xftst0
	./xftst1
	./xftst2
	./xftst3
	./xftst4
	./xftst5
	./xftst6
	./xftst7

ibuild : xitst0 xitst1 xitst2 xitst3 xitst4 xitst5 xitst6 xitst7 xitst8 xitst9
irun : ibuild
	./xitst0
	./xitst1
	./xitst2
	./xitst3
	./xitst4
	./xitst5
	./xitst6
	./xitst7
	./xitst8
	./xitst9


xitst9 : mitst9.o itst9.o
	$(CC) $(CCFLAGS) -o $@ mitst9.o itst9.o
xitst8 : mitst8.o itst8.o
	$(CC) $(CCFLAGS) -o $@ mitst8.o itst8.o
xitst7 : mitst7.o itst7.o
	$(CC) $(CCFLAGS) -o $@ mitst7.o itst7.o
xitst6 : mitst6.o itst6.o
	$(CC) $(CCFLAGS) -o $@ mitst6.o itst6.o
xitst5 : mitst5.o itst5.o
	$(CC) $(CCFLAGS) -o $@ mitst5.o itst5.o
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
xftst3 : mftst3.o ftst3.o
	$(CC) $(CCFLAGS) -o $@ mftst3.o ftst3.o
xftst4 : mftst4.o ftst4.o
	$(CC) $(CCFLAGS) -o $@ mftst4.o ftst4.o
xftst5 : mftst5.o ftst5.o
	$(CC) $(CCFLAGS) -o $@ mftst5.o ftst5.o
xftst6 : mftst6.o ftst6.o
	$(CC) $(CCFLAGS) -o $@ mftst6.o ftst6.o
xftst7 : mftst7.o ftst7.o
	$(CC) $(CCFLAGS) -o $@ mftst7.o ftst7.o
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
