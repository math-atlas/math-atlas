#include /home/grads/whaley/AT3.7/ATLAS3.7.8/Make.Linux_P4SSE2
#include /home/rwhaley/AT3.7/ATLAS3.7.6.2/Make.Linux_P4ESSE3_2
#include /home/rwhaley/AT3.7/ATLAS3.7.9.1/Make.Linux_P4ESSE3
#include /home/rwhaley/AT3.7/ATLAS3.7.9.1/Make.Linux_gcc
#include /opt/whaley/TEST/ATLAS3.7.9.0/Make.Linux_P4SSE2
#include /home/rwhaley/AT/ATLAS3.7.9.1/Make.Linux_icc32
#include /home/rwhaley/AT/ATLAS3.7.9.1/Make.Linux_HAMMER64SSE2
#include /home/rwhaley/TEST/ATLAS3.7.8.2/Make.Linux_EfficeonSSE2
#include /home/rwhaley/TEST/ATLAS3.7.9.0/Make.Linux_EfficeonSSE2

#include /home/msujon/ATLAS/ATLAS3.9.67/Linux_X8632_AVX/Make.Linux_X8632_AVX
#include /home/msujon/ATLAS/ATLAS3.9.67/Linux_X8664_AVX/Make.Linux_X8664_AVX
#include /home/msujon/ATLAS/ATLAS3.9.67/Linux_X8632_SSE/Make.Linux_X8632_SSE
#include /home/msujon/ATLAS/ATLAS3.9.67/Linux_X8664_SSE/Make.Linux_X8664_SSE

include /home/msujon/ATLAS/ATLAS3.9.77/Linux_X8664_AVX/Make.Linux_X8664_AVX
#include /home/msujon/ATLAS/ATLAS3.10.0/Linux_X8664_AVX/Make.Linux_X8664_AVX


#KFLAGS=-V 
KFLAGS=-V -Ps b A 0 1 -P all 0 128 -U 2
#KFLAGS=-V -Ps b A 0 1 -P all 0 128 
pre=d
rout=dot
N=800
file=fko_tt.s
URT=DOT
FKO = ../bin/fko
test=1
uopt=
#uopt="-X 1 -Y 1 -Fx 16 -Fy 16"
#uopt="-X 1 -Y 1 -Fx 32 -Fy 32"

#all: dot axpy copy swap scal iamax #asum
all: dot axpy copy swap scal iamax 

dot: sdot ddot
axpy: saxpy daxpy
asum: sasum dasum
copy: scopy dcopy
swap: sswap dswap
scal: sscal dscal
iamax: siamax diamax
amax: samax damax

sdot : $(FKO)
	$(MAKE) build pre=s rout=dot URT=`echo dot | tr 'a-z' 'A-Z'` \
	   			     KFLAGS="$(KFLAGS)"
	./TestTime.py s dot $(file) $(N) $(test) $(uopt)
ddot : $(FKO)
	$(MAKE) build pre=d rout=dot URT=`echo dot | tr 'a-z' 'A-Z'` \
	   			     KFLAGS="$(KFLAGS)"
	./TestTime.py d dot $(file) $(N) $(test) $(uopt)

saxpy: $(FKO)
	$(MAKE) build pre=s rout=axpy URT=`echo axpy | tr 'a-z' 'A-Z'` \
	   			      KFLAGS="$(KFLAGS)"
	./TestTime.py s axpy $(file) $(N) $(test) $(uopt)
daxpy: $(FKO)
	$(MAKE) build pre=d rout=axpy URT=`echo axpy | tr 'a-z' 'A-Z'` \
	   			      KFLAGS="$(KFLAGS)"
	./TestTime.py d axpy $(file) $(N) $(test) $(uopt)

sasum: $(FKO)
	$(MAKE) build pre=s rout=asum URT=`echo asum | tr 'a-z' 'A-Z'` \
	   			      KFLAGS="$(KFLAGS)"
	./TestTime.py s asum $(file) $(N) $(test) $(uopt)
dasum: $(FKO)
	$(MAKE) build pre=d rout=asum URT=`echo asum | tr 'a-z' 'A-Z'` \
	   			      KFLAGS="$(KFLAGS)"
	./TestTime.py d asum $(file) $(N) $(test) $(uopt)
scopy: $(FKO)
	$(MAKE) build pre=s rout=copy URT=`echo copy | tr 'a-z' 'A-Z'` \
	   			      KFLAGS="$(KFLAGS)"
	./TestTime.py s copy $(file) $(N) $(test) $(uopt)
dcopy: $(FKO)
	$(MAKE) build pre=d rout=copy URT=`echo copy | tr 'a-z' 'A-Z'` \
	   			      KFLAGS="$(KFLAGS)"
	./TestTime.py d copy $(file) $(N) $(test) $(uopt)
sswap: $(FKO)
	$(MAKE) build pre=s rout=swap URT=`echo swap | tr 'a-z' 'A-Z'` \
	   			      KFLAGS="$(KFLAGS)"
	./TestTime.py s swap $(file) $(N) $(test) $(uopt)
dswap: $(FKO)
	$(MAKE) build pre=d rout=swap URT=`echo swap | tr 'a-z' 'A-Z'` \
	   			      KFLAGS="$(KFLAGS)"
	./TestTime.py d swap $(file) $(N) $(test) $(uopt)
sscal: $(FKO)
	$(MAKE) build pre=s rout=scal URT=`echo scal | tr 'a-z' 'A-Z'` \
	   			      KFLAGS="$(KFLAGS)"
	./TestTime.py s scal $(file) $(N) $(test) $(uopt)
dscal: $(FKO)
	$(MAKE) build pre=d rout=scal URT=`echo scal | tr 'a-z' 'A-Z'` \
	   			      KFLAGS="$(KFLAGS)"
	./TestTime.py d scal $(file) $(N) $(test) $(uopt)

siamax: $(FKO)
	$(MAKE) build pre=s rout=iamax URT=`echo iamax | tr 'a-z' 'A-Z'` \
	   			      KFLAGS="$(KFLAGS)"
	./TestTime.py s iamax $(file) $(N) $(test) $(uopt)

diamax: $(FKO)
	$(MAKE) build pre=d rout=iamax URT=`echo iamax | tr 'a-z' 'A-Z'` \
	   			      KFLAGS="$(KFLAGS)"
	./TestTime.py d iamax $(file) $(N) $(test) $(uopt)

samax: $(FKO)
	$(MAKE) build pre=s rout=amax URT=`echo amax | tr 'a-z' 'A-Z'` \
	   			      KFLAGS="$(KFLAGS)"
	./TestTime.py s amax $(file) $(N) $(test) $(uopt)

damax: $(FKO)
	$(MAKE) build pre=d rout=amax URT=`echo amax | tr 'a-z' 'A-Z'` \
	   			      KFLAGS="$(KFLAGS)"
	./TestTime.py d amax $(file) $(N) $(test) $(uopt)

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
