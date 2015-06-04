include ../../Make.inc

libd = ../lib
incd = ../include

CC = gcc
CCFL = $(CCFLAGS) -Wall -I$(incd)
AR = ar
ARF = r
RANLIB = echo

lib = $(libd)/libfkoinfo.a
obj = FKO_GetArchInfo.o FKO_DestroyArchInfo.o FKO_NumRegs.o FKO_Veclen.o \
      FKO_RegtypesAliased.o FKO_CachelineSize.o FKO_HasSpecialInst.o \
      FKO_PipelenADD.o FKO_PipelenMUL.o FKO_PipelenMAC.o FKO_PipelenDIV.o \
      FKO_TypestrToInt.o  FKO_InststrToInt.o FKO_ParseInfoLine.o \
      FKO_GetOptLoopInfo.o
incs = $(libd)/fko_parseinfo.h $(libd)/fko_hinfoC.h $(libd)/fko_hinfo.h

lib : $(lib)
$(lib) : $(obj)
	$(AR) $(ARF) $(lib) $(obj)
	$(RANLIB) $(lib)

clean :
	- rm -f $(obj)

.c.o: $(incs)
	$(CC) $(CCFL) -c $<
