#include /home/whalley-scratch/whaley/AT3.5/ATLAS3.6.0_0/Make.Linux_P4SSE2
#include /home/rwhaley/AT3.7/ATLAS3.7.3/Make.Linux_P4ESSE3
include /tmp/rwhaley/ATLAS3.7.3.0/Make.Linux_P4SSE2

pre=d
rout=dot
URT = DOT
kern=error
cache=
NN="N=80000"
KFLAG=
topt=-X 1 1
lower=hello
# upper=`echo $(lower) | tr '[a-z]' 'A-Z'`
upper=`cat tt.in`

test:
	echo $(upper)
xtimel1 : timel1.o
	$(CC) -o xtimel1 timel1.o

timel1.o : timel1.c
	$(CC) $(CCFLAGS) -c timel1.c

l1time :
	$(MAKE) l1time0 rout=$(rout) URT=`echo $(rout) | tr 'a-z' 'A-Z'` \
           kern=$(kern) cache="$(cache)" NN="$(NN)" KFLAG="$(KFLAG)" \
           topt="$(topt)" pre=$(pre)
l1time0 : 
	- rm -f ../blas/$(pre)$(rout).s $(L1Tdir)/fko.tmp if2.tmp
	cd ../blas ; $(MAKE) $(pre)$(rout).s KFLAG="$(KFLAG)"
	cp ../blas/$(pre)$(rout).s $(L1Tdir)/../$(URT)/fkorout.c
	cd $(L1Tdir) ; make $(pre)$(rout)case urout=fkorout.c UCC=gcc \
           UCCFLAGS="-x assembler-with-cpp" $(cache) $(NN) > fko.tmp 2>&1
	cat $(L1Tdir)/fko.tmp
	$(MAKE) l1test pre=$(pre) rout=$(rout) cache=$(cache) N=$(NN)
	@echo "****\n\n"
	fgrep -e 'time=' -e 'mflop=' $(L1Tdir)/fko.tmp > ifk2.tmp
	fgrep -e 'time=' -e 'mflop=' $(L1Tdir)/fko.tmp

l1test :
	cd $(L1Tdir) ; make $(pre)$(rout)test urout=fkorout.c UCC=gcc \
           UCCFLAGS="-x assembler-with-cpp" $(cache) $(NN) opt="$(topt)"
../blas/$(pre)$(rout).s :
