SHELL = /bin/tcsh
CC = gcc
CCFLAGS = -g
objs = y.tab.o lex.yy.o h2l.o l2a.o inst.o symtab.o arch.o misc.o flow.o \
       bvec.o vars.o optreg.o optloop.o
HDIR = ../include
heads= $(HDIR)/fko_arch.h $(HDIR)/fko_inst.h $(HDIR)/fko_misc.h \
       $(HDIR)/fko_types.h $(HDIR)/fko_h2l.h $(HDIR)/fko_l2a.h \
       $(HDIR)/fko_symtab.h $(HDIR)/fko.h $(HDIR)/fko_flow.h \
       $(HDIR)/fko_bvec.h $(HDIR)/fko_vars.h $(HDIR)/fko_optreg.h \
       $(HDIR)/fko_loop.h

fko : $(heads) fko.o $(objs)
	$(CC) $(CCFLAGS) -o $@ fko.o $(objs) -lfl
#	$(CC) $(CCFLAGS) -o $@ fko.o $(objs) -ll

xprintinst : ../include/fko.h ../include/fko_inst.h ../src/printinst.c
	$(CC) $(CCFLAGS) -I../include -o $@ ../src/printinst.c
xprintregs : $(heads) ../src/printregs.c
	$(CC) $(CCFLAGS) -I../include -o $@ ../src/printregs.c

#arch.o : $(heads) arch.c $(heads)
#	$(CC) $(CCFLAGS) -c arch.c

y.tab.h : $(heads) ../src/hil_gram.y
	yacc -d -v ../src/hil_gram.y
y.tab.c : $(heads) ../src/hil_gram.y
	yacc -v ../src/hil_gram.y
lex.yy.c : y.tab.h ../src/hil_lex.l
	lex ../src/hil_lex.l
lex.yy.o : $(heads) y.tab.h lex.yy.c
	$(CC) -I../bin -I../include $(CCFLAGS) -c lex.yy.c
y.tab.o : $(heads) y.tab.h
	$(CC) -I../include $(CCFLAGS) -c y.tab.c

optloop.o : $(heads) ../src/optloop.c
	$(CC) -I../include $(CCFLAGS) -c ../src/optloop.c
optreg.o : $(heads) ../src/optreg.c
	$(CC) -I../include $(CCFLAGS) -c ../src/optreg.c
vars.o : $(heads) ../src/vars.c
	$(CC) -I../include $(CCFLAGS) -c ../src/vars.c
bvec.o : $(heads) ../src/bvec.c
	$(CC) -I../include $(CCFLAGS) -c ../src/bvec.c
flow.o : $(heads) ../src/flow.c
	$(CC) -I../include $(CCFLAGS) -c ../src/flow.c
misc.o : $(heads) ../src/misc.c
	$(CC) -I../include $(CCFLAGS) -c ../src/misc.c
arch.o : $(heads) ../src/arch.c
	$(CC) -I../include $(CCFLAGS) -c ../src/arch.c
symtab.o : $(heads) ../src/symtab.c
	$(CC) -I../include $(CCFLAGS) -c ../src/symtab.c
inst.o : $(heads) ../src/inst.c
	$(CC) -I../include $(CCFLAGS) -c ../src/inst.c
l2a.o : $(heads) ../src/l2a.c
	$(CC) -I../include $(CCFLAGS) -c ../src/l2a.c
h2l.o : $(heads) ../src/h2l.c
	$(CC) -I../include $(CCFLAGS) -c ../src/h2l.c
fko.o : $(heads) ../src/fko.c
	$(CC) -I../include $(CCFLAGS) -c ../src/fko.c

clean :
	- rm -f $(objs) y.tab.h y.tab.c lex.yy.c fko.o y.output
