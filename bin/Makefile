SHELL = /bin/tcsh
CC = gcc
CCFLAGS = -g
objs = y.tab.o lex.yy.o h2l.o l2a.o inst.o symtab.o arch.o misc.o
HDIR = ../include
heads= $(HDIR)/fko_arch.h $(HDIR)/fko_inst.h $(HDIR)/fko_misc.h \
       $(HDIR)/fko_types.h $(HDIR)/fko_h2l.h $(HDIR)/fko_l2a.h \
       $(HDIR)/fko_symtab.h $(HDIR)/ifko.h

ifko : $(heads) ifko.o $(objs)
	/usr/local/rational/releases/PurifyPlusFamily.2002a.06.00/sun4_solaris2/bin/purify $(CC) $(CCFLAGS) -o $@ ifko.o $(objs) -ll

xtst3 :  tst3.o fexample3.o
	$(CC) $(CCFLAGS) -o $@ tst3.o fexample3.o
xtst2 :  tst2.o fexample.o
	$(CC) $(CCFLAGS) -o $@ tst2.o fexample.o
tst.o : tst.c
	$(CC) $(CCFLAGS) -c tst.c
xtst :  tst.o example.o
	$(CC) $(CCFLAGS) -o $@ tst.o example.o
tst2.o : tst2.c
	$(CC) $(CCFLAGS) -c tst2.c
tst3.o : tst3.c
	$(CC) $(CCFLAGS) -c tst3.c
example.o : example.s
	$(CC) $(CCFLAGS) -c example.s
example.s : ifko example.b
	./ifko example.b >! example.s
fexample3.o : fexample3.s
	$(CC) $(CCFLAGS) -c fexample3.s
fexample3.s : ifko fexample3.b
	./ifko fexample3.b >! fexample3.s
fexample.o : fexample.s
	$(CC) $(CCFLAGS) -c fexample.s
fexample.s : ifko fexample.b
	./ifko fexample.b >! fexample.s

xprintinst : ../include/ifko.h ../include/fko_inst.h ../src/printinst.c
	$(CC) $(CCFLAGS) -I../include -o $@ ../src/printinst.c

#arch.o : $(heads) arch.c $(heads)
#	$(CC) $(CCFLAGS) -c arch.c

y.tab.c : y.tab.h
y.tab.h : ../src/hil_gram.y $(heads)
	yacc -v -d ../src/hil_gram.y
lex.yy.c : y.tab.h ../src/hil_lex.l
	lex ../src/hil_lex.l
lex.yy.o : $(heads) lex.yy.c
	$(CC) -I../include $(CCFLAGS) -c lex.yy.c
y.tab.o : $(heads) y.tab.c
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
	rm -f $(objs) y.tab.h y.tab.c lex.yy.c ifko.o y.output
