#include /home/grads/whaley/AT3.7/ATLAS3.7.6.0/Make.Linux_P4SSE2
#include /home/rwhaley/TEST/ATLAS3.7.8.2/Make.Linux_EfficeonSSE2
include /home/rwhaley/AT3.7/ATLAS3.7.6.2/Make.Linux_P4ESSE3_2

pre=d
rout=dot
URT = DOT
kern=error
cache=
NN="N=80000"
KFLAGS= -V -U 4
opt =-X 1 -Y 1 -Fx 16 -Fy 16
topt=-X 1 1 -Y 1 1 -Fx 16 -Fy 16
lower=hello
# upper=`echo $(lower) | tr '[a-z]' 'A-Z'`
# upper=`cat tt.in`

test:
	echo $(upper)
xtimel1 : timel1.o
	$(CC) -o xtimel1 timel1.o

timel1.o : timel1.c
	$(CC) $(CCFLAGS) -c timel1.c

l1time :
	$(MAKE) l1time0 rout=$(rout) URT=`echo $(rout) | tr 'a-z' 'A-Z'` \
           kern=$(kern) cache="$(cache)" NN="$(NN)" KFLAGS="$(KFLAGS)" \
           topt="$(topt)" opt="$(opt)" pre=$(pre)
l1time0 : 
	- rm -f ../blas/$(pre)$(rout).s $(L1Tdir)/fko.tmp if2.tmp
	cd ../blas ; $(MAKE) $(pre)$(rout).s KFLAGS="$(KFLAGS)"
	cp ../blas/$(pre)$(rout).s $(L1Tdir)/../$(URT)/fkorout.c
	cd $(L1Tdir) ; make $(pre)$(rout)case urout=fkorout.c UCC=gcc \
           UCCFLAGS="-x assembler-with-cpp" opt="$(opt) $(cache)" \
           $(NN) > fko.tmp 2>&1
	cat $(L1Tdir)/fko.tmp
	$(MAKE) l1test pre=$(pre) rout=$(rout) N=$(NN)
	@echo "****\n\n"
	fgrep -e 'time=' -e 'mflop=' $(L1Tdir)/fko.tmp > ifk2.tmp
	fgrep -e 'time=' -e 'mflop=' $(L1Tdir)/fko.tmp

l1test :
	cd $(L1Tdir) ; make $(pre)$(rout)test urout=fkorout.c UCC=gcc \
           UCCFLAGS="-x assembler-with-cpp" $(NN) opt="$(topt)"
../blas/$(pre)$(rout).s :
