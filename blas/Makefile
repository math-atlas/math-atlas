SHELL=/bin/sh
FKO = ../bin/fko
KFLAG=
#scal.s : scal.b
#	../bin/fko $(KFLAG) -o scal.s scal.b
clean :
	rm -f *.s

build_comp:
	cd ../bin ; $(MAKE) fko

%.s : %.b
	$(MAKE) build_comp
	$(FKO) $(KFLAG) $< -o $@
