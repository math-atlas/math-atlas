CC = gcc
CCFLAGS = -O3
mydir=`pwd`
srcdir=./TEST
topd = $(mydir)/AtlasBase
defs = -def topd $(topd) -def ext $(mydir)/$(srcdir)/xextract

$(srcdir) :
	mkdir TEST
	$(CC) $(CCFLAGS) -o TEST/xextract extract.c
	./TEST/xextract -langM -b AtlasBase/make.base -o TEST/Makefile \
                  rout=Make.atldir $(defs)
src : $(srcdir)
	cd $(srcdir) ; make
tarfile: src
	cd $(srcdir) ; ./atltar.sh

killall :
	rm -rf $(srcdir)
