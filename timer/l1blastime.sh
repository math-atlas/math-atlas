#!/bin/bash

#save the runtime, it will use as id in every logfiles
scripttime=`date +%Y%m%d_%H%M%S`
echo $scripttime

#set all paths. 
FKOPATH=/home/msujon/iFKO
TIMEdir=/home/msujon/iFKO/timer
SCRIPTdir=/home/msujon/iFKO/scripts

ATLASdir=/home/msujon/ALTAS
ATLVER=ATLAS3.9.77
ATLOBJdir=Linux_X8664_AVX

FKOLIBdir=$TIMEdir/lib
mkdir $FKOLIBdir
#create log, result and src dir
for dir in blas_src blas_result blas_log
do
   mkdir $TIMEdir/$dir
   mkdir $TIMEdir/$dir/$scripttime
done

RESULTPATH=$TIMEdir/blas_result
SRCPATH=$TIMEdir/blas_src/$scripttime
LOGPATH=$TIMEdir/blas_log/$scripttime

echo "Result Path: " $RESULTPATH
echo "Source Path: " $SRCPATH
echo "Library Path: " $FKOLIBdir
echo "Logfiles Path: " $LOGPATH
echo "FKO Path: " $FKOPATH

# delete old lib files if exists
rm -f $FKOLIBdir/*

# parameter to create library
CC=gcc
CFLAGS="-fomit-frame-pointer -mfpmath=sse -mfma4 -O2 -fno-tree-loop-optimize \
        -msse4.2 -mfma4 -m64"

SMLIB=sblas_fko.a
DMLIB=dblas_fko.a
OBJ=

#set configuration parameters
copt=noflushing
N=16000
C=1
R="7 copy swap scal asum amax axpy dot" 
F=200

if [ $# -ge 1 ] 
then
   N=$1
fi

#echo $N

for pre in s d
do
   for blas in swap copy asum axpy dot scal iamax
   do
      echo "Tuning " $pre$blas "... ... ..."
      echo "-------------------------------"
      inputlog=$LOGPATH/$pre${blas}_${N}_${scripttime}
      $SCRIPTdir/ifko.py $blas $pre $N > $inputlog
      lflag=`tail -n 9 $inputlog | head -n 1`
      iflag=`echo $lflag | cut -d' ' -f 6- -s`
      fline=`tail -n 5 $inputlog | head -n 1`
      perf=`echo $fline | cut -d' ' -f 4 -s`
      echo $pre$blas " : " $perf " [ " $iflag " ] " 
      echo $pre$blas ":" $perf >> $RESULTPATH/result_$scripttime
      echo "     " $iflag >> $RESULTPATH/result_$scripttime
      #regenarate the assembly using the opt flags
      $FKOPATH/bin/fko $iflag -o $SRCPATH/$pre${blas}.s $FKOPATH/blas/$pre$blas.b
      #cerate obj using the assemblies
      $CC $CFLAGS -c $SRCPATH/$pre${blas}.s
      OBJ="$OBJ $pre${blas}.o"
   done
   #create library for desired prefix
   ar r $FKOLIBdir/${pre}${MLIB} $OBJ
   OBJ=
done

#######################################################
# now its time run and timer
########################################################
cd $TIMEdir 
make clean

make xsl1blastst_ifko
$TIMEdir/xsl1blastst_ifko "-C $C -n $N -X 3 1 1 1 -R $R /
-F $F"

make xdl1blastst_ifko
$TIMEdir/xdl1blastst_ifko "-C $C -n $N -X 3 1 1 1 -R $R /
-F $F"


