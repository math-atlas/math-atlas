SHELL = /bin/tcsh

CC = gcc
CCFLAGS = -g -m32
#CCFLAGS = -g -m64
IFKO = ../bin/fko
KFLAGS = -U 4 -v

all : ibuild fbuild dbuild hbuild svbuild irun frun drun hrun svrun

svbuild : xsvtst0 xsvtst1 xsvtst2 xsvtst3 xsvtst4 xsvtst5
svrun:
	./xsvtst0
	./xsvtst1
	./xsvtst2
	./xsvtst3
	./xsvtst4
	./xsvtst5
xsvtst0 : msvtst0.o svtst0.o
	$(CC) $(CCFLAGS) -o $@ msvtst0.o svtst0.o
xsvtst1 : msvtst1.o svtst1.o
	$(CC) $(CCFLAGS) -o $@ msvtst1.o svtst1.o
xsvtst2 : msvtst2.o svtst2.o
	$(CC) $(CCFLAGS) -o $@ msvtst2.o svtst2.o
xsvtst3 : msvtst3.o svtst3.o
	$(CC) $(CCFLAGS) -o $@ msvtst3.o svtst3.o
xsvtst4 : msvtst4.o svtst4.o
	$(CC) $(CCFLAGS) -o $@ msvtst4.o svtst4.o
xsvtst5 : msvtst5.o svtst5.o
	$(CC) $(CCFLAGS) -o $@ msvtst5.o svtst5.o

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

fbuild :xftst0 xftst1 xftst2 xftst3 xftst4 xftst5 xftst6 xftst7 xftst8 xftst9 \
        xftst10 xftst11
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
	./xftst10
	./xftst11

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
xftst10 : mftst10.o ftst10.o
	$(CC) $(CCFLAGS) -o $@ mftst10.o ftst10.o
xftst11 : mftst11.o ftst11.o
	$(CC) $(CCFLAGS) -o $@ mftst11.o ftst11.o

dbuild :xdtst0 xdtst1 xdtst2 xdtst3 xdtst4 xdtst5 xdtst6 xdtst7 xdtst8 xdtst9 \
        xdtst10 xdtst11
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
	./xdtst10
	./xdtst11

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
xdtst10 : mdtst10.o dtst10.o
	$(CC) $(CCFLAGS) -o $@ mdtst10.o dtst10.o
xdtst11 : mdtst11.o dtst11.o
	$(CC) $(CCFLAGS) -o $@ mdtst11.o dtst11.o

ibuild :xitst0 xitst1 xitst2 xitst3 xitst4 xitst5 xitst6 xitst7 xitst8 xitst9 \
        xitst10 xitst11 xitst12 xitst13 xitst14 xitst15 xitst16
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
	./xitst13
	./xitst14
	./xitst15
	./xitst16

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
xitst13 : mitst13.o itst13.o
	$(CC) $(CCFLAGS) -o $@ mitst13.o itst13.o
xitst14 : mitst14.o itst14.o
	$(CC) $(CCFLAGS) -o $@ mitst14.o itst14.o
xitst15 : mitst15.o itst15.o
	$(CC) $(CCFLAGS) -o $@ mitst15.o itst15.o
xitst16 : mitst16.o itst16.o
	$(CC) $(CCFLAGS) -o $@ mitst16.o itst16.o


mitst3.c : mitst0.c
	ln -s mitst0.c mitst3.c
clean:
	rm -f x* *.o *.s
build_comp:
	cd ../bin ; $(MAKE) fko

%.o : %.s
	$(CC) $(CCFLAGS) -c -o $@ $<
%.s : %.b
	$(MAKE) build_comp
	$(IFKO) $(KFLAGS) $< > $@
%.o : %.c
	$(CC) $(CCFLAGS) -c -o $@ $<
