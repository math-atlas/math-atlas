#!/bin/sh
rm -f ../ifko.tar.bz2
cd .. 
cp -f extract.c iFKO/src/.
tar cvf ifko.tar iFKO/configure iFKO/fko_atlconf iFKO/LICENSE.txt \
    iFKO/base/*.base iFKO/include/*.h iFKO/src/*c \
    iFKO/FKO/README iFKO/FKO/include/*.h iFKO/FKO/src/*.c \
    iFKO/FKO/src/hil_gram.y iFKO/FKO/src/hil_lex.l 
bzip2 ifko.tar
mv ifko.tar.bz2 iFKO/.
cd iFKO
