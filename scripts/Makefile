#include /home/grads/whaley/AT3.7/ATLAS3.7.8/Make.Linux_P4SSE2
#include /home/rwhaley/AT3.7/ATLAS3.7.6.2/Make.Linux_P4ESSE3_2
include /home/rwhaley/AT3.7/ATLAS3.7.9.1/Make.Linux_P4ESSE3
#include /home/rwhaley/AT3.7/ATLAS3.7.9.1/Make.Linux_gcc
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

go : $(FKO)
	$(MAKE) build URT=`echo $(rout) | tr 'a-z' 'A-Z'` KFLAGS="$(KFLAGS)"
	./TestTime.py $(pre) $(rout) $(file) $(N) $(test) $(uopt)
build :
	$(FKO) $(KFLAGS) -o $(L1Tdir)/../$(URT)/$(file) \
               ../blas/$(pre)$(rout).b
	
build_comp : 
	$(MAKE) $(FKO)
force_build:
$(FKO) : force_build
	cd ../bin ; $(MAKE) fko
