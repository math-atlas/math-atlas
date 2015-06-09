#!/bin/sh
rm -f ../fko.tar.bz2
cd .. 
tar cvf fko.tar FKO/bin/Makefile FKO/blas/*.b  FKO/blas/Makefile \
   extern/include/*.h \
   extern/src/FKO_*.c extern/src/query*.c extern/src/Makefile
