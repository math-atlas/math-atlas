#include /home/msujon/ATLAS/ATLAS3.9.77/Linux_X8664_AVX/bin/Make.inc
include /home/msujon/ATLAS/ATLAS3.9.74/Linux_X8664_AVX/bin/Make.inc

#mySRCdir = /home/msujon/iFKO/timer/
mySRCdir = $(CURDIR)
acmld = /opt/acml5.1.0/

#----------------------------
# Set reference library here 
#----------------------------
#FBLASlib = $(LIBdir)/libf77refblas.a
FBLASlib = $(acmld)/gfortran64/lib/libacml.a

BLASlib = $(FBLASlib)

#--------------------------------
# Set level1 blas library of ifko
#--------------------------------

FKOdir = /home/msujon/iFKO
Blib = $(mySRCdir)/lib
sFKOlib = $(Blib)/sblas_fko.a $(Blib)/snrm2_fko.a
dFKOlib = $(Blib)/dblas_fko.a $(Blib)/dnrm2_fko.a

all: xsl1blastst_ifko xdl1blastst_ifko

xsl1blastst_ifko : sl1blastst_ifko.o sl1lib ststlib
	$(FLINKER) $(FCLINKFLAGS) -o $@ sl1blastst_ifko.o $(sFKOlib) \
                   $(TESTlib) $(BLASlib) $(ATLASlib) $(LIBS)

xdl1blastst_ifko : dl1blastst_ifko.o dl1lib dtstlib
	$(FLINKER) $(FCLINKFLAGS) -o $@ dl1blastst_ifko.o $(dFKOlib) \
                   $(TESTlib) $(BLASlib) $(ATLASlib) $(LIBS)

sl1blastst_ifko.o : $(mySRCdir)/l1blastst.c $(INCSdir)/atlas_misc.h \
                    $(INCSdir)/atlas_tst.h $(INCSdir)/atlas_level1.h
	$(ICC) -o $@ -c $(ICCFLAGS) -DSREAL -DIFKO -DALIGN32  $(mySRCdir)/l1blastst.c


dl1blastst_ifko.o : $(mySRCdir)/l1blastst.c $(INCSdir)/atlas_misc.h \
                   $(INCSdir)/atlas_tst.h $(INCSdir)/atlas_level1.h
	$(ICC) -o $@ -c $(ICCFLAGS) -DDREAL -DIFKO -DALIGN32 $(mySRCdir)/l1blastst.c

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


