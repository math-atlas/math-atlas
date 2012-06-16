include /home/msujon/ATLAS/ATLAS3.9.77/Linux_X8664_AVX/bin/Make.inc
#mySRCdir = /home/msujon/iFKO/timer/
mySRCdir = $(CURDIR)

FKOdir = /home/msujon/iFKO
Blib = $(mySRCdir)/lib
sFKOlib = $(Blib)/sblas_fko.a
dFKOlib = $(Blib)/dblas_fko.a

xsl1blastst_ifko : sl1blastst_ifko.o sl1lib ststlib
	$(FLINKER) $(FCLINKFLAGS) -o $@ sl1blastst_ifko.o $(sFKOlib) \
                   $(TESTlib) $(BLASlib) $(ATLASlib) $(LIBS)

xdl1blastst_ifko : dl1blastst_ifko.o dl1lib dtstlib
	$(FLINKER) $(FCLINKFLAGS) -o $@ dl1blastst_ifko.o $(dFKOlib) \
                   $(TESTlib) $(BLASlib) $(ATLASlib) $(LIBS)

sl1blastst_ifko.o : $(mySRCdir)/l1blastst.c $(INCSdir)/atlas_misc.h \
                    $(INCSdir)/atlas_tst.h $(INCSdir)/atlas_level1.h
	$(KC) -o $@ -c $(KCFLAGS) -DSREAL -DIFKO  $(mySRCdir)/l1blastst.c


dl1blastst_ifko.o : $(mySRCdir)/l1blastst.c $(INCSdir)/atlas_misc.h \
                   $(INCSdir)/atlas_tst.h $(INCSdir)/atlas_level1.h
	$(KC) -o $@ -c $(KCFLAGS) -DDREAL -DIFKO  $(mySRCdir)/l1blastst.c

ssl1lib: sl1lib
sl1lib: auxlib
	cd $(L1Bdir) ; $(MAKE) slvl1
sststlib: ststlib
ststlib: 
	cd $(TSTdir) ; $(MAKE) slib

dl1lib: auxlib
	cd $(L1Bdir) ; $(MAKE) dlvl1
dtstlib: 
	cd $(TSTdir) ; $(MAKE) dlib

auxlib : force_build
	cd $(AUXdir) ; $(MAKE) lib

force_build:
clean:
	rm -f x* *.o *.grd


