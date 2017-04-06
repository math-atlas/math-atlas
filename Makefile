CC = gcc
CCFLAGS = -O3
mydir=`pwd`
srcdir=./TEST
topd = $(mydir)/AtlasBase
defs = -def topd $(topd) -def ext $(mydir)/$(srcdir)/xextract
OS = $(shell uname -s)
ifeq ($(OS),MINGW32_NT-6.1)
CCFLAGS += -Wl,--stack,16777216
endif

$(srcdir) :
	mkdir $(srcdir)
	$(CC) $(CCFLAGS) -o $(srcdir)/xextract extract.c
	$(srcdir)/xextract -langM -b AtlasBase/make.base -o $(srcdir)/Makefile \
                  rout=Make.atldir $(defs)
src : $(srcdir)
	cd $(srcdir) ; make
tarfile: src
	cd $(srcdir) ; ./atltar.sh

killall :
	rm -rf $(srcdir)
