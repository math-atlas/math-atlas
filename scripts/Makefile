include /opt/whaley/TEST/ATLAS3.7.9.0/Make.Linux_P4SSE2

KFLAGS=
pre=d
rout=dot
N=80000
file=fko_tt.s
URT=DOT
FKO = ../bin/fko
test=1

go : $(FKO)
	$(MAKE) build URT=`echo $(rout) | tr 'a-z' 'A-Z'`
	./TestTime.py $(pre) $(rout) $(file) $(N) $(test)
build :
	$(FKO) $(KFLAGS) -o $(L1Tdir)/../$(URT)/$(file) \
               ../blas/$(pre)$(rout).b
	
build_comp : $(FKO)
$(FKO) : 
	cd ../bin ; $(MAKE) fko
