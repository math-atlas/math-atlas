SHELL = /bin/tcsh
CC = gcc
CCFLAGS = -mcpu=ultrasparc -g
IFKO = ../bin/ifko

all : ibuild fbuild dbuild hbuild irun frun drun hrun

hbuild :xhtst0 xhtst1 xhtst2 xhtst3 xhtst4
hrun:
	./xhtst0
	./xhtst1
	./xhtst2
	./xhtst3
	./xhtst4

xhtst0 : mhtst0.o htst0.o
	$(CC) $(CCFLAGS) -o $@ mhtst0.o htst0.o
xhtst1 : mhtst1.o htst1.o
	$(CC) $(CCFLAGS) -o $@ mhtst1.o htst1.o
xhtst2 : mhtst2.o htst2.o
	$(CC) $(CCFLAGS) -o $@ mhtst2.o htst2.o
xhtst3 : mhtst3.o htst3.o
	$(CC) $(CCFLAGS) -o $@ mhtst3.o htst3.o
xhtst4 : mhtst4.o htst4.o
	$(CC) $(CCFLAGS) -o $@ mhtst4.o htst4.o

fbuild :xftst0 xftst1 xftst2 xftst3 xftst4 xftst5 xftst6 xftst7 xftst8 xftst9
frun:
	./xftst0
	./xftst1
	./xftst2
	./xftst3
	./xftst4
	./xftst5
	./xftst6
	./xftst7
	./xftst8
	./xftst9

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
xftst8 : mftst8.o ftst8.o
	$(CC) $(CCFLAGS) -o $@ mftst8.o ftst8.o
xftst9 : mftst9.o ftst9.o
	$(CC) $(CCFLAGS) -o $@ mftst9.o ftst9.o

dbuild :xdtst0 xdtst1 xdtst2 xdtst3 xdtst4 xdtst5 xdtst6 xdtst7 xdtst8 xdtst9
drun:
	./xdtst0
	./xdtst1
	./xdtst2
	./xdtst3
	./xdtst4
	./xdtst5
	./xdtst6
	./xdtst7
	./xdtst8
	./xdtst9

xdtst0 : mdtst0.o dtst0.o
	$(CC) $(CCFLAGS) -o $@ mdtst0.o dtst0.o
xdtst1 : mdtst1.o dtst1.o
	$(CC) $(CCFLAGS) -o $@ mdtst1.o dtst1.o
xdtst2 : mdtst2.o dtst2.o
	$(CC) $(CCFLAGS) -o $@ mdtst2.o dtst2.o
xdtst3 : mdtst3.o dtst3.o
	$(CC) $(CCFLAGS) -o $@ mdtst3.o dtst3.o
xdtst4 : mdtst4.o dtst4.o
	$(CC) $(CCFLAGS) -o $@ mdtst4.o dtst4.o
xdtst5 : mdtst5.o dtst5.o
	$(CC) $(CCFLAGS) -o $@ mdtst5.o dtst5.o
xdtst6 : mdtst6.o dtst6.o
	$(CC) $(CCFLAGS) -o $@ mdtst6.o dtst6.o
xdtst7 : mdtst7.o dtst7.o
	$(CC) $(CCFLAGS) -o $@ mdtst7.o dtst7.o
xdtst8 : mdtst8.o dtst8.o
	$(CC) $(CCFLAGS) -o $@ mdtst8.o dtst8.o
xdtst9 : mdtst9.o dtst9.o
	$(CC) $(CCFLAGS) -o $@ mdtst9.o dtst9.o

ibuild :xitst0 xitst1 xitst2 xitst3 xitst4 xitst5 xitst6 xitst7 xitst8 xitst9 \
        xitst10 xitst11 xitst12
irun:
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
	./xitst10
	./xitst11
	./xitst12

xitst0 : mitst0.o itst0.o
	$(CC) $(CCFLAGS) -o $@ mitst0.o itst0.o
xitst1 : mitst1.o itst1.o
	$(CC) $(CCFLAGS) -o $@ mitst1.o itst1.o
xitst2 : mitst2.o itst2.o
	$(CC) $(CCFLAGS) -o $@ mitst2.o itst2.o
xitst3 : mitst3.o itst3.o
	$(CC) $(CCFLAGS) -o $@ mitst3.o itst3.o
xitst4 : mitst4.o itst4.o
	$(CC) $(CCFLAGS) -o $@ mitst4.o itst4.o
xitst5 : mitst5.o itst5.o
	$(CC) $(CCFLAGS) -o $@ mitst5.o itst5.o
xitst6 : mitst6.o itst6.o
	$(CC) $(CCFLAGS) -o $@ mitst6.o itst6.o
xitst7 : mitst7.o itst7.o
	$(CC) $(CCFLAGS) -o $@ mitst7.o itst7.o
xitst8 : mitst8.o itst8.o
	$(CC) $(CCFLAGS) -o $@ mitst8.o itst8.o
xitst9 : mitst9.o itst9.o
	$(CC) $(CCFLAGS) -o $@ mitst9.o itst9.o
xitst10 : mitst10.o itst10.o
	$(CC) $(CCFLAGS) -o $@ mitst10.o itst10.o
xitst11 : mitst11.o itst11.o
	$(CC) $(CCFLAGS) -o $@ mitst11.o itst11.o
xitst12 : mitst12.o itst12.o
	$(CC) $(CCFLAGS) -o $@ mitst12.o itst12.o


mitst3.c : mitst0.c
	ln -s mitst0.c mitst3.c
clean:
	rm -f x* *.o *.s
build_comp:
	cd ../bin ; $(MAKE) ifko

%.o : %.s
	$(CC) $(CCFLAGS) -c -o $@ $<
%.s : %.b
	$(MAKE) build_comp
	$(IFKO) $< > $@
%.o : %.c
	$(CC) $(CCFLAGS) -c -o $@ $<
