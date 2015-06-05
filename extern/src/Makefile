include ../../Make.inc

libd = ../lib
incd = ../include
CCFL = $(CCFLAGS) -Wall -I$(incd)
ext = extract

incs = $(incd)/fko_parseinfo.h $(incd)/fko_infoC.h $(incd)/fko_info.h
lib = $(libd)/libfkoinfo.a
obj = FKO_CachelineSize.o FKO_DestroyArchInfo.o FKO_DestroyInstInfo.o \
      FKO_DestroyOptLoopInfo.o FKO_GetArchInfo.o FKO_GetInstInfo.o \
      FKO_GetOptLoopInfo.o FKO_HasSpecialInst.o FKO_InststrToInt.o \
      FKO_NumRegs.o FKO_ParseInfoLine.o FKO_PipelenADD.o FKO_PipelenDIV.o \
      FKO_PipelenMAC.o FKO_PipelenMUL.o FKO_RegtypesAliased.o \
      FKO_TypestrToInt.o FKO_Veclen.o

$(lib) : $(obj)
	$(AR) $(ARF) $(lib) $(obj)
	$(RANLIB) $(lib)

lib : $(lib)

clean :
	- rm -f $(obj)

FKO_GetArchInfo.o : $(incs) FKO_GetArchInfo.c
	$(CC) $(CCFL) -o FKO_GetArchInfo.o -c FKO_GetArchInfo.c
FKO_DestroyArchInfo.o : $(incs) FKO_DestroyArchInfo.c
	$(CC) $(CCFL) -o FKO_DestroyArchInfo.o -c FKO_DestroyArchInfo.c
FKO_NumRegs.o : $(incs) FKO_NumRegs.c
	$(CC) $(CCFL) -o FKO_NumRegs.o -c FKO_NumRegs.c
FKO_Veclen.o : $(incs) FKO_Veclen.c
	$(CC) $(CCFL) -o FKO_Veclen.o -c FKO_Veclen.c
FKO_RegtypesAliased.o : $(incs) FKO_RegtypesAliased.c
	$(CC) $(CCFL) -o FKO_RegtypesAliased.o -c FKO_RegtypesAliased.c
FKO_CachelineSize.o : $(incs) FKO_CachelineSize.c
	$(CC) $(CCFL) -o FKO_CachelineSize.o -c FKO_CachelineSize.c
FKO_HasSpecialInst.o : $(incs) FKO_HasSpecialInst.c
	$(CC) $(CCFL) -o FKO_HasSpecialInst.o -c FKO_HasSpecialInst.c
FKO_PipelenADD.o : $(incs) FKO_PipelenADD.c
	$(CC) $(CCFL) -o FKO_PipelenADD.o -c FKO_PipelenADD.c
FKO_PipelenMUL.o : $(incs) FKO_PipelenMUL.c
	$(CC) $(CCFL) -o FKO_PipelenMUL.o -c FKO_PipelenMUL.c
FKO_PipelenMAC.o : $(incs) FKO_PipelenMAC.c
	$(CC) $(CCFL) -o FKO_PipelenMAC.o -c FKO_PipelenMAC.c
FKO_PipelenDIV.o : $(incs) FKO_PipelenDIV.c
	$(CC) $(CCFL) -o FKO_PipelenDIV.o -c FKO_PipelenDIV.c
FKO_TypestrToInt.o : $(incs) FKO_TypestrToInt.c
	$(CC) $(CCFL) -o FKO_TypestrToInt.o -c FKO_TypestrToInt.c
FKO_InststrToInt.o : $(incs) FKO_InststrToInt.c
	$(CC) $(CCFL) -o FKO_InststrToInt.o -c FKO_InststrToInt.c
FKO_ParseInfoLine.o : $(incs) FKO_ParseInfoLine.c
	$(CC) $(CCFL) -o FKO_ParseInfoLine.o -c FKO_ParseInfoLine.c
FKO_GetOptLoopInfo.o : $(incs) FKO_GetOptLoopInfo.c
	$(CC) $(CCFL) -o FKO_GetOptLoopInfo.o -c FKO_GetOptLoopInfo.c
FKO_DestroyOptLoopInfo.o : $(incs) FKO_DestroyOptLoopInfo.c
	$(CC) $(CCFL) -o FKO_DestroyOptLoopInfo.o -c FKO_DestroyOptLoopInfo.c
FKO_GetInstInfo.o : $(incs) FKO_GetInstInfo.c
	$(CC) $(CCFL) -o FKO_GetInstInfo.o -c FKO_GetInstInfo.c
FKO_DestroyInstInfo.o : $(incs) FKO_DestroyInstInfo.c
	$(CC) $(CCFL) -o FKO_DestroyInstInfo.o -c FKO_DestroyInstInfo.c

Makefile : ../../ifko.base
	$(ext) -b ../../ifko.base -langM -o Makefile rout=Make.extern
