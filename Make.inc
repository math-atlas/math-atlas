SHELL=/bin/sh
BLDdir=/home/whaley/TEST/Debug/ifko
TOPdir=/home/whaley/TEST/iFKO/
CC=gcc
CDEFS=-DAVX2=1
CFLAGS=-Wall -g
INCs=-I$(TOPdir)/include -I$(TOPdir)/extern/include
CCFLAGS=$(CFLAGS) $(CDEFS) $(INCs)
AR=ar
ARF=r
RANLIB=echo
