SHELL=/bin/sh
IFKO = ../bin/ifko
KFLAG=
#scal.s : scal.b
#	../bin/ifko $(KFLAG) -o scal.s scal.b
clean :
	rm -f *.s

build_comp:
	cd ../bin ; $(MAKE) ifko

%.s : %.b
	$(MAKE) build_comp
	$(IFKO) $(KFLAG) $< -o $@
