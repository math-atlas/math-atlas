#include /home/whalley-scratch/whaley/AT3.5/ATLAS3.6.0_0/Make.Linux_P4SSE2
include /home/rwhaley/AT3.7/ATLAS3.7.3/Make.Linux_P4ESSE3

pre=d
rout=dot
URT = DOT
kern=error
cache=
N="N=80000"

xtimel1 : timel1.o
	$(CC) -o xtimel1 timel1.o

timel1.o : timel1.c
	$(CC) $(CCFLAGS) -c timel1.c

l1time :
	cp $(kern) $(L1Tdir)/../$(URT)/fkorout.c
	- rm -f $(L1Tdir)/fko.tmp if2.tmp
	cd $(L1Tdir) ; make $(pre)$(rout)case urout=fkorout.c UCC=gcc \
           UCCFLAGS="-x assembler-with-cpp" $(cache) $(N) > fko.tmp 2>&1
	cat $(L1Tdir)/fko.tmp
	echo "****\n\n"
	fgrep -e 'time=' -e 'mflop=' $(L1Tdir)/fko.tmp > ifk2.tmp
	fgrep -e 'time=' -e 'mflop=' $(L1Tdir)/fko.tmp
