#include /home/grads/whaley/AT3.7/ATLAS3.7.8/Make.Linux_P4SSE2
#include /home/rwhaley/AT3.7/ATLAS3.7.6.2/Make.Linux_P4ESSE3_2
#include /home/rwhaley/AT3.7/ATLAS3.7.9.1/Make.Linux_P4ESSE3
#include /home/rwhaley/AT3.7/ATLAS3.7.9.1/Make.Linux_gcc

#include /home/msujon/ATLAS/ATLAS/MYBUILD/Make.inc
#include /home/msujon/ATLAS/ATLAS3.9.67/Linux_X8632_SSE/Make.Linux_X8632_SSE
include /home/msujon/ATLAS/ATLAS3.9.67/Linux_X8664_SSE/Make.Linux_X8664_SSE

#include /opt/whaley/TEST/ATLAS3.7.9.0/Make.Linux_P4SSE2
#include /home/rwhaley/AT/ATLAS3.7.9.1/Make.Linux_icc32
#include /home/rwhaley/AT/ATLAS3.7.9.1/Make.Linux_HAMMER64SSE2
#include /home/rwhaley/TEST/ATLAS3.7.8.2/Make.Linux_EfficeonSSE2
#include /home/rwhaley/TEST/ATLAS3.7.9.0/Make.Linux_EfficeonSSE2



KFLAGS= 
pre=d
rout=dot
N=80000
file=fko_tt.s
URT=DOT
FKO = ../bin/fko
test=1
uopt=

dot: sdot ddot

sdot : $(FKO)
	$(MAKE) build pre=s URT=`echo dot | tr 'a-z' 'A-Z'` KFLAGS="$(KFLAGS)"
	./TestTime.py s dot $(file) $(N) $(test) $(uopt)
ddot : $(FKO)
	$(MAKE) build pre=d URT=`echo dot | tr 'a-z' 'A-Z'` KFLAGS="$(KFLAGS)"
	./TestTime.py d dot $(file) $(N) $(test) $(uopt)

daxpy: $(FKO)
	$(MAKE) build pre=d rout=axpy URT=`echo axpy | tr 'a-z' 'A-Z'` KFLAGS="$(KFLAGS)"
	./TestTime.py d axpy $(file) $(N) $(test) $(uopt)


go : $(FKO)
	$(MAKE) build URT=`echo $(rout) | tr 'a-z' 'A-Z'` KFLAGS="$(KFLAGS)"
	./TestTime.py $(pre) $(rout) $(file) $(N) $(test) $(uopt)
build :
	$(FKO) $(KFLAGS) -o $(L1Tdir)/$(URT)/$(file) \
               ../blas/$(pre)$(rout).b
	
build_comp : 
	$(MAKE) $(FKO)
force_build:
$(FKO) : force_build
	cd ../bin ; $(MAKE) fko
