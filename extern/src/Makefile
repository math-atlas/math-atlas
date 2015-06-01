shell = /bin/bash
libd = ../lib
incd = ../include

CC = gcc
CCFLAGS = -Wall -g -O0 -I$(incd)
AR = ar
ARF = r
RANLIB = echo

lib = $(libd)/libfkoinfo.a
obj = fko_ParseArchInfo.o fko_ParseInfoLine.o
incs = $(libd)/fko_parseinfo.h $(libd)/fko_archinfoC.h

lib : $(lib)
$(lib) : $(obj)
	$(AR) $(ARF) $(lib) $(obj)
	$(RANLIB) $(lib)

clean :
	- rm -f $(obj)

.c.o: 
	$(CC) $(CCFLAGS) -c $<
