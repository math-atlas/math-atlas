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

#include /home/msujon/ATLAS/ATLAS3.9.77/Linux_X8664_AVX/Make.Linux_X8664_AVX

#include /home/msujon/ATLAS/ATLAS3.10.0/Linux_X8664_AVX_FMA/Make.Linux_X8664_AVX_FMA

include /home/msujon/ATLAS/ATLAS3.11.14/Linux_X8664_AVX_FMA/Make.Linux_X8664_AVX_FMA


KFLAGS=-V 
#KFLAGS=-V -Ps b A 0 1 -P all 0 128 -U 2
#KFLAGS=-V -Ps b A 0 1 -P all 0 128 
pre=d
rout=dot
#N=80000
N=16000
M=16000
lda=$M
NB=64
beta=1
file=fko_tt.s
URT=DOT
FKO = ../bin/fko
test=1
uopt=
#uopt="-X 1 -Y 1 -Fx 16 -Fy 16"
#uopt="-X 1 -Y 1 -Fx 8 -Fy 8"
uopt="-C 1 -X 1 -Y 1 -Fx 32 -Fy 32"
opt="-Fx 32 -Fy 32 -Fa 32"
optn="-Fa 32 -Fy 32"
#uopt="-C 1000000 -X 1 -Y 1 -Fx 32 -Fy 32"

# target directory.. by default level1 
Tdir=$(L1Tdir)

#all: dot axpy copy swap scal iamax asum amax
all: dot axpy copy swap scal iamax asum amax sin cos

dot: sdot ddot
axpy: saxpy daxpy
asum: sasum dasum
copy: scopy dcopy
swap: sswap dswap
scal: sscal dscal
iamax: siamax diamax
amax: samax damax
nrm2: snrm2 dnrm2

sin: ssin dsin
cos: scos dcos

irk1amax: sirk1amax dirk1amax
irk2amax: sirk2amax dirk2amax
irk3amax: sirk3amax dirk3amax

gemvt: sgemvt dgemvt


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

snrm2: $(FKO)
	$(MAKE) build pre=s rout=nrm2 URT=`echo nrm2 | tr 'a-z' 'A-Z'` \
	   			      KFLAGS="$(KFLAGS)"
	./TestTime.py s nrm2 $(file) $(N) $(test) $(uopt)

dnrm2: $(FKO)
	$(MAKE) build pre=d rout=nrm2 URT=`echo nrm2 | tr 'a-z' 'A-Z'` \
	   			      KFLAGS="$(KFLAGS)"
	./TestTime.py d nrm2 $(file) $(N) $(test) $(uopt)


ssin: $(FKO)
	$(MAKE) build pre=s rout=sin URT=`echo sin | tr 'a-z' 'A-Z'` \
	   			      KFLAGS="$(KFLAGS)"
	./TestTime.py s sin $(file) $(N) $(test) $(uopt)

dsin: $(FKO)
	$(MAKE) build pre=d rout=sin URT=`echo sin | tr 'a-z' 'A-Z'` \
	   			      KFLAGS="$(KFLAGS)"
	./TestTime.py d sin $(file) $(N) $(test) $(uopt)

scos: $(FKO)
	$(MAKE) build pre=s rout=cos URT=`echo cos | tr 'a-z' 'A-Z'` \
	   			      KFLAGS="$(KFLAGS)"
	./TestTime.py s cos $(file) $(N) $(test) $(uopt)

dcos: $(FKO)
	$(MAKE) build pre=d rout=cos URT=`echo cos | tr 'a-z' 'A-Z'` \
	   			      KFLAGS="$(KFLAGS)"
	./TestTime.py d cos $(file) $(N) $(test) $(uopt)

sirk1amax: $(FKO)
	$(MAKE) build pre=s rout=irk1amax URT=`echo irk1amax | tr 'a-z' 'A-Z'` \
	   			      KFLAGS="$(KFLAGS)"
	./TestTime.py s irk1amax $(file) $(N) $(test) $(uopt)

dirk1amax: $(FKO)
	$(MAKE) build pre=d rout=irk1amax URT=`echo irk1amax | tr 'a-z' 'A-Z'` \
	   			      KFLAGS="$(KFLAGS)"
	./TestTime.py d irk1amax $(file) $(N) $(test) $(uopt)

sirk2amax: $(FKO)
	$(MAKE) build pre=s rout=irk2amax URT=`echo irk2amax | tr 'a-z' 'A-Z'` \
	   			      KFLAGS="$(KFLAGS)"
	./TestTime.py s irk2amax $(file) $(N) $(test) $(uopt)

dirk2amax: $(FKO)
	$(MAKE) build pre=d rout=irk2amax URT=`echo irk2amax | tr 'a-z' 'A-Z'` \
	   			      KFLAGS="$(KFLAGS)"
	./TestTime.py d irk2amax $(file) $(N) $(test) $(uopt)

sirk3amax: $(FKO)
	$(MAKE) build pre=s rout=irk3amax URT=`echo irk3amax | tr 'a-z' 'A-Z'` \
	   			      KFLAGS="$(KFLAGS)"
	./TestTime.py s irk3amax $(file) $(N) $(test) $(uopt)

dirk3amax: $(FKO)
	$(MAKE) build pre=d rout=irk3amax URT=`echo irk3amax | tr 'a-z' 'A-Z'` \
	   			      KFLAGS="$(KFLAGS)"
	./TestTime.py d irk3amax $(file) $(N) $(test) $(uopt)

dgemvt: $(FKO)
	$(MAKE) build pre=d rout=gemvt URT=`echo mvtcases | tr 'a-z' 'A-Z'` \
	   			      KFLAGS="$(KFLAGS)" Tdir="$(MVTdir)"
	./TestTime2.py d gemvt $(file) $(N) $(M) $(lda) $(test) $(opt)
dgemvn: $(FKO)
	$(MAKE) build pre=d rout=gemvn URT=`echo mvncases | tr 'a-z' 'A-Z'` \
	   			      KFLAGS="$(KFLAGS)" Tdir="$(MVTdir)"
	./TestTime2.py d gemvn $(file) $(N) $(M) $(lda) $(test) $(opt)

dgemvt_2d: $(FKO)
	$(MAKE) build pre=d rout=gemvt_2d URT=`echo mvtcases | tr 'a-z' 'A-Z'` \
	   			      KFLAGS="$(KFLAGS)" Tdir="$(MVTdir)"
	./TestTime2.py d gemvt $(file) $(N) $(M) $(lda) $(test) $(opt)

dgemvt2: $(FKO)
	$(MAKE) build pre=d rout=gemvt2 URT=`echo mvtcases | tr 'a-z' 'A-Z'` \
	   			      KFLAGS="$(KFLAGS)" Tdir="$(MVTdir)"
	./TestTime2.py d gemvt $(file) $(N) $(M) $(lda) $(test) $(opt)

dgemvt2_2d: $(FKO)
	$(MAKE) build pre=d rout=gemvt2_2d URT=`echo mvtcases | tr 'a-z' 'A-Z'` \
	   			      KFLAGS="$(KFLAGS)" Tdir="$(MVTdir)"
	./TestTime2.py d gemvt $(file) $(N) $(M) $(lda) $(test) $(opt)

dgemvt4: $(FKO)
	$(MAKE) build pre=d rout=gemvt4 URT=`echo mvtcases | tr 'a-z' 'A-Z'` \
	   			      KFLAGS="$(KFLAGS)" Tdir="$(MVTdir)"
	./TestTime2.py d gemvt $(file) $(N) $(M) $(lda) $(test) $(opt)

dgemvt4_2d: $(FKO)
	$(MAKE) build pre=d rout=gemvt4_2d URT=`echo mvtcases | tr 'a-z' 'A-Z'` \
	   			      KFLAGS="$(KFLAGS)" Tdir="$(MVTdir)"
	./TestTime2.py d gemvt $(file) $(N) $(M) $(lda) $(test) $(opt)

dgemvt6: $(FKO)
	$(MAKE) build pre=d rout=gemvt6 URT=`echo mvtcases | tr 'a-z' 'A-Z'` \
	   			      KFLAGS="$(KFLAGS)" Tdir="$(MVTdir)"
	./TestTime2.py d gemvt $(file) $(N) $(M) $(lda) $(test) $(opt)

dgemvt6_2d: $(FKO)
	$(MAKE) build pre=d rout=gemvt6_2d URT=`echo mvtcases | tr 'a-z' 'A-Z'` \
	   			      KFLAGS="$(KFLAGS)" Tdir="$(MVTdir)"
	./TestTime2.py d gemvt $(file) $(N) $(M) $(lda) $(test) $(opt)

dgemvt7: $(FKO)
	$(MAKE) build pre=d rout=gemvt7 URT=`echo mvtcases | tr 'a-z' 'A-Z'` \
	   			      KFLAGS="$(KFLAGS)" Tdir="$(MVTdir)"
	./TestTime2.py d gemvt $(file) $(N) $(M) $(lda) $(test) $(opt)

dgemvt8: $(FKO)
	$(MAKE) build pre=d rout=gemvt8 URT=`echo mvtcases | tr 'a-z' 'A-Z'` \
	   			      KFLAGS="$(KFLAGS)" Tdir="$(MVTdir)"
	./TestTime2.py d gemvt $(file) $(N) $(M) $(lda) $(test) $(opt)

dger1: $(FKO)
	$(MAKE) build pre=d rout=ger1 URT=`echo r1cases | tr 'a-z' 'A-Z'` \
	   			      KFLAGS="$(KFLAGS)" Tdir="$(R1Tdir)"
	./TestTime2.py d ger1 $(file) $(N) $(M) $(lda) $(test) $(opt)

dger2: $(FKO)
	$(MAKE) build pre=d rout=ger2 URT=`echo r2cases | tr 'a-z' 'A-Z'` \
	   			      KFLAGS="$(KFLAGS)" Tdir="$(R1Tdir)"
	./TestTime2.py d ger2 $(file) $(N) $(M) $(lda) $(test) $(opt)

dgemm: $(FKO)
	$(MAKE) build pre=d rout=gemm URT=`echo cases | tr 'a-z' 'A-Z'` \
	   			      KFLAGS="$(KFLAGS)" Tdir="$(MMTdir)"
	./TestTime3.py d gemm $(file) $(NB) $(beta) $(test) 

dgemm4x1: $(FKO)
	$(MAKE) build pre=d rout=gemm4x1 URT=`echo cases | tr 'a-z' 'A-Z'` \
	   			      KFLAGS="$(KFLAGS)" Tdir="$(MMTdir)"
	./TestTime3.py d gemm $(file) $(NB) $(beta) $(test) 

dgemm2x2: $(FKO)
	$(MAKE) build pre=d rout=gemm2x2 URT=`echo cases | tr 'a-z' 'A-Z'` \
	   			      KFLAGS="$(KFLAGS)" Tdir="$(MMTdir)"
	./TestTime3.py d gemm $(file) $(NB) $(beta) $(test) 

go : $(FKO)
	$(MAKE) build URT=`echo $(rout) | tr 'a-z' 'A-Z'` KFLAGS="$(KFLAGS)"
	./TestTime.py $(pre) $(rout) $(file) $(test) $(uopt)

build :
	$(FKO) $(KFLAGS) -o $(Tdir)/$(URT)/$(file) \
               ../blas/$(pre)$(rout).b
	
build_comp : 
	$(MAKE) $(FKO)
force_build:
$(FKO) : force_build
	cd ../bin ; $(MAKE) fko
