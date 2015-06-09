include ../../Make.inc

incd = $(TOPdir)/extern/include
srcd = $(TOPdir)/extern/src
libd = ../lib
ext = $(EXTRACT)

incs = $(incd)/fko_parseinfo.h $(incd)/fko_infoC.h $(incd)/fko_info.h
lib = $(libd)/libfkoinfo.a
obj = FKO_CachelineSize.o FKO_DestroyArchInfo.o FKO_DestroyInstInfo.o \
      FKO_DestroyOptLoopInfo.o FKO_GetArchInfo.o FKO_GetInstInfo.o \
      FKO_GetOptLoopInfo.o FKO_HasSpecialInst.o FKO_InststrToInt.o \
      FKO_NumRegs.o FKO_ParseInfoLine.o FKO_PipelenADD.o FKO_PipelenDIV.o \
      FKO_PipelenMAC.o FKO_PipelenMUL.o FKO_RegtypesAliased.o \
      FKO_TypestrToInt.o FKO_Veclen.o FKO_system.o

$(lib) : $(obj)
	$(AR) $(ARF) $(lib) $(obj)
	$(RANLIB) $(lib)

lib : $(lib)

clean :
	- rm -f $(obj)

FKO_GetArchInfo.o : $(incs) $(srcd)/FKO_GetArchInfo.c
	$(CC) $(CCFLAGS) -o FKO_GetArchInfo.o -c $(srcd)/FKO_GetArchInfo.c
FKO_DestroyArchInfo.o : $(incs) $(srcd)/FKO_DestroyArchInfo.c
	$(CC) $(CCFLAGS) -o FKO_DestroyArchInfo.o -c $(srcd)/FKO_DestroyArchInfo.c
FKO_NumRegs.o : $(incs) $(srcd)/FKO_NumRegs.c
	$(CC) $(CCFLAGS) -o FKO_NumRegs.o -c $(srcd)/FKO_NumRegs.c
FKO_Veclen.o : $(incs) $(srcd)/FKO_Veclen.c
	$(CC) $(CCFLAGS) -o FKO_Veclen.o -c $(srcd)/FKO_Veclen.c
FKO_RegtypesAliased.o : $(incs) $(srcd)/FKO_RegtypesAliased.c
	$(CC) $(CCFLAGS) -o FKO_RegtypesAliased.o -c $(srcd)/FKO_RegtypesAliased.c
FKO_CachelineSize.o : $(incs) $(srcd)/FKO_CachelineSize.c
	$(CC) $(CCFLAGS) -o FKO_CachelineSize.o -c $(srcd)/FKO_CachelineSize.c
FKO_HasSpecialInst.o : $(incs) $(srcd)/FKO_HasSpecialInst.c
	$(CC) $(CCFLAGS) -o FKO_HasSpecialInst.o -c $(srcd)/FKO_HasSpecialInst.c
FKO_PipelenADD.o : $(incs) $(srcd)/FKO_PipelenADD.c
	$(CC) $(CCFLAGS) -o FKO_PipelenADD.o -c $(srcd)/FKO_PipelenADD.c
FKO_PipelenMUL.o : $(incs) $(srcd)/FKO_PipelenMUL.c
	$(CC) $(CCFLAGS) -o FKO_PipelenMUL.o -c $(srcd)/FKO_PipelenMUL.c
FKO_PipelenMAC.o : $(incs) $(srcd)/FKO_PipelenMAC.c
	$(CC) $(CCFLAGS) -o FKO_PipelenMAC.o -c $(srcd)/FKO_PipelenMAC.c
FKO_PipelenDIV.o : $(incs) $(srcd)/FKO_PipelenDIV.c
	$(CC) $(CCFLAGS) -o FKO_PipelenDIV.o -c $(srcd)/FKO_PipelenDIV.c
FKO_TypestrToInt.o : $(incs) $(srcd)/FKO_TypestrToInt.c
	$(CC) $(CCFLAGS) -o FKO_TypestrToInt.o -c $(srcd)/FKO_TypestrToInt.c
FKO_InststrToInt.o : $(incs) $(srcd)/FKO_InststrToInt.c
	$(CC) $(CCFLAGS) -o FKO_InststrToInt.o -c $(srcd)/FKO_InststrToInt.c
FKO_ParseInfoLine.o : $(incs) $(srcd)/FKO_ParseInfoLine.c
	$(CC) $(CCFLAGS) -o FKO_ParseInfoLine.o -c $(srcd)/FKO_ParseInfoLine.c
FKO_GetOptLoopInfo.o : $(incs) $(srcd)/FKO_GetOptLoopInfo.c
	$(CC) $(CCFLAGS) -o FKO_GetOptLoopInfo.o -c $(srcd)/FKO_GetOptLoopInfo.c
FKO_DestroyOptLoopInfo.o : $(incs) $(srcd)/FKO_DestroyOptLoopInfo.c
	$(CC) $(CCFLAGS) -o FKO_DestroyOptLoopInfo.o -c $(srcd)/FKO_DestroyOptLoopInfo.c
FKO_GetInstInfo.o : $(incs) $(srcd)/FKO_GetInstInfo.c
	$(CC) $(CCFLAGS) -o FKO_GetInstInfo.o -c $(srcd)/FKO_GetInstInfo.c
FKO_DestroyInstInfo.o : $(incs) $(srcd)/FKO_DestroyInstInfo.c
	$(CC) $(CCFLAGS) -o FKO_DestroyInstInfo.o -c $(srcd)/FKO_DestroyInstInfo.c
FKO_system.o : $(incs) $(srcd)/FKO_system.c
	$(CC) $(CCFLAGS) -o FKO_system.o -c $(srcd)/FKO_system.c

Makefile : $(TOPdir)/ifko.base
	$(ext) -b $(TOPdir)/ifko.base -langM -o Makefile rout=Make.extern
