SHELL=/bin/sh
FKO = ../bin/fko
#KFLAGS = -U 2
KFLAGS = -V -G 1 1 DN
#scal.s : scal.b
#	../bin/fko $(KFLAG) -o scal.s scal.b
clean :
	rm -f *.s dot.out

build_comp:
	cd ../bin ; $(MAKE) fko

%.s : %.b
	$(MAKE) build_comp
	$(FKO) $(KFLAGS) $< -o $@
