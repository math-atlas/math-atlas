SHELL = /bin/tcsh
CC = gcc
CCFLAGS = -g
objs = y.tab.o lex.yy.o h2l.o l2a.o inst.o symtab.o arch.o misc.o
HDIR = ../include
heads= $(HDIR)/fko_arch.h $(HDIR)/fko_inst.h $(HDIR)/fko_misc.h \
       $(HDIR)/fko_types.h $(HDIR)/fko_h2l.h $(HDIR)/fko_l2a.h \
       $(HDIR)/fko_symtab.h $(HDIR)/ifko.h

ifko : $(heads) ifko.o $(objs)
	$(CC) $(CCFLAGS) -o $@ ifko.o $(objs) -ll

xprintinst : ../include/ifko.h ../include/fko_inst.h ../src/printinst.c
	$(CC) $(CCFLAGS) -I../include -o $@ ../src/printinst.c

#arch.o : $(heads) arch.c $(heads)
#	$(CC) $(CCFLAGS) -c arch.c

y.tab.h : $(heads) ../src/hil_gram.y
	yacc -d -v ../src/hil_gram.y
y.tab.c : $(heads) ../src/hil_gram.y
	yacc -v ../src/hil_gram.y
lex.yy.c : y.tab.h ../src/hil_lex.l
	lex ../src/hil_lex.l
lex.yy.o : $(heads) y.tab.h lex.yy.c
	$(CC) -I../include $(CCFLAGS) -c lex.yy.c
y.tab.o : $(heads) y.tab.h
	$(CC) -I../include $(CCFLAGS) -c y.tab.c

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
ifko.o : $(heads) ../src/ifko.c
	$(CC) -I../include $(CCFLAGS) -c ../src/ifko.c

clean :
	- rm -f $(objs) y.tab.h y.tab.c lex.yy.c ifko.o y.output
