CC = gcc
CCFLAGS = -O3
mydir=`pwd`
srcdir=./TEST
topd = $(mydir)/AtlasBase
defs = -def topd $(topd) -def ext $(srcdir)/xextract

tarfile: src
	cd $(srcdir) ; ./atltar.sh
src : $(srcdir)
	cd $(srcdir) ; make

$(srcdir) :
	mkdir TEST
	$(CC) $(CCFLAGS) -o TEST/xextract extract.c
	./TEST/xextract -langM -b AtlasBase/make.base -o TEST/Makefile \
                  rout=Make.atldir $(defs)
killall :
	rm -rf $(srcdir)
