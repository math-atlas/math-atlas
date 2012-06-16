#!/bin/bash

#save the runtime, it will use as id in every logfiles
scripttime=`date +%Y%m%d_%H%M%S`
echo $scripttime

#set all paths. 
FKOPATH=/home/msujon/iFKO
TIMEdir=/home/msujon/iFKO/timer
SCRIPTdir=/home/msujon/iFKO/scripts


FKOLIBdir=$TIMEdir/lib
mkdir $FKOLIBdir

CMPPATH=$TIMEdir/result
mkdir $CMPPATH

#create log, result and src dir
for dir in blas_src blas_result blas_log
do
   mkdir $TIMEdir/$dir
   mkdir $TIMEdir/$dir/$scripttime
done

RESULTPATH=$TIMEdir/blas_result
SRCPATH=$TIMEdir/blas_src/$scripttime
LOGPATH=$TIMEdir/blas_log/$scripttime

echo "FKO Path: " $FKOPATH
echo "Result Path: " $RESULTPATH
echo "Source Path: " $SRCPATH
echo "Library Path: " $FKOLIBdir
echo "Logfiles Path: " $LOGPATH
echo "Comparison-result Path: " $CMPPATH
echo ""

# delete old lib files if exists
rm -f -r $FKOLIBdir/*

# parameter to create library
CC=gcc
CFLAGS="-fomit-frame-pointer -mfpmath=sse -mfma4 -O2 -fno-tree-loop-optimize \
        -msse4.2 -mfma4 -m64"

#SMLIB=sblas_fko.a
#DMLIB=dblas_fko.a
MLIB=blas_fko.a
OBJ=

#set configuration parameters
copt=noflushing
N=16384
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
      echo "Tuning " ${pre}blas "... ... ..."
      echo "==============================="
   for blas in swap copy asum axpy dot scal iamax
   do
      inputlog=$LOGPATH/$pre${blas}_${N}_${scripttime}
      $SCRIPTdir/ifko.py $blas $pre $N > $inputlog
      lflag=`tail -n 9 $inputlog | head -n 1`
      iflag=`echo $lflag | cut -d' ' -f 6- -s`
      fline=`tail -n 5 $inputlog | head -n 1`
      perf=`echo $fline | cut -d' ' -f 4 -s`
      echo $pre${blas}": "${perf}" ["${iflag}"]" 
      echo $pre$blas ":" $perf >> $RESULTPATH/result_$scripttime
      echo "     " $iflag >> $RESULTPATH/result_$scripttime
      #regenarate the assembly using the opt flags
      $FKOPATH/bin/fko $iflag -o $SRCPATH/$pre${blas}.s $FKOPATH/blas/$pre$blas.b
      #cerate obj using the assemblies
      $CC $CFLAGS -o $SRCPATH/$pre${blas}.o -c $SRCPATH/$pre${blas}.s
      OBJ="$OBJ $SRCPATH/$pre${blas}.o"
   done
   echo ""
   #create library for desired prefix
   ar r $FKOLIBdir/${pre}${MLIB} $OBJ
   OBJ=
   echo ""
done

#######################################################
# now its time run and timer
########################################################

CMP=acml

cd $TIMEdir 
make clean

make xsl1blastst_ifko

`$TIMEdir/xsl1blastst_ifko "-C $C -n $N -X 3 1 1 1 -R $R -F $F"` > $CMPPATH/sl1blas_${CMP}_ifko_$scripttime

make xdl1blastst_ifko

`$TIMEdir/xdl1blastst_ifko "-C $C -n $N -X 3 1 1 1 -R $R -F $F"` > $CMPPATH/dl1blas_${CMP}_ifko_$scripttime

echo " SL1BLAS: $CMP vs. ifko"
echo " ======================="
cat $CMPPATH/sl1blas_${CMP}_ifko_$scripttime

echo " DL1BLAS: $CMP vs. ifko"
echo " ======================="
cat $CMPPATH/dl1blas_${CMP}_ifko_$scripttime

