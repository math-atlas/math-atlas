#!/bin/bash
#
# save the runtime, it will use as id in every logfiles
#
scripttime=`date +%Y%m%d_%H%M%S`
echo $scripttime
#
# set all paths. 
#
#FKOPATH=/home/msujon/iFKO
FKOPATH=/home/msujon/Research/working/iFKO-Norm2-SSV/iFKO
TIMEdir=${FKOPATH}/timer
SCRIPTdir=${FKOPATH}/scripts

FKOLIBdir=$TIMEdir/lib
mkdir $FKOLIBdir

CMPPATH=$TIMEdir/result
mkdir $CMPPATH

LOGdir=$TIMEdir/log
mkdir $LOGdir
#
# create log, result and src dir
#
for dir in l1blas_src l1blas_result l1blas_log
do
   mkdir $LOGdir/$dir
   mkdir $LOGdir/$dir/$scripttime
done

RESULTPATH=$LOGdir/l1blas_result
SRCPATH=$LOGdir/l1blas_src/$scripttime
LOGPATH=$LOGdir/l1blas_log/$scripttime

echo "FKO Path: " $FKOPATH
echo "Result Path: " $RESULTPATH
echo "Source Path: " $SRCPATH
echo "Library Path: " $FKOLIBdir
echo "Logfiles Path: " $LOGPATH
echo "Comparison-result Path: " $CMPPATH
echo ""
#
# delete old lib files if exists
#
rm -f -r $FKOLIBdir/*
#
# parameter to create library
#
CC=gcc
CFLAGS="-fomit-frame-pointer -mfpmath=sse -mfma4 -O2 -fno-tree-loop-optimize \
        -msse4.2 -mfma4 -m64"

#SMLIB=sblas_fko.a
#DMLIB=dblas_fko.a
MLIB=blas_fko.a
NRM2LIB=nrm2_fko.a

OBJ=
NRM2OBJ=

#
# set configuration parameters
#
copt=noflushing
N=16384
#C=8388
C=1
R="8 copy swap scal asum amax axpy dot nrm2" 
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
   for blas in swap copy asum axpy dot scal iamax nrm2
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
#
#     regenarate the assembly using the opt flags
#

#     
#     NOTE: Right now, we need to handle nrm2 separately. nrm2.b in blas dir
#     actually represents ssq.b and nrm2.c is the stub for nrm2. we rename 
#     nrm2.b from ssq.b just to simpify the use of python scripts
#     NOTE: we need special nrm2test and nrm2time to time norm2 
#
      if [ $blas = nrm2 ]
      then
         $FKOPATH/bin/fko $iflag -o $SRCPATH/${pre}ssq.s \
            $FKOPATH/blas/$pre$blas.b
         $CC $CFLAGS -o $SRCPATH/${pre}ssq.o -c $SRCPATH/${pre}ssq.s
         cp $FKOPATH/blas/${pre}nrm2.c $SRCPATH/${pre}nrm2.c
         $CC $CFLAGS -o $SRCPATH/${pre}nrm2.o -c $SRCPATH/${pre}nrm2.c
         NRM2OBJ="$SRCPATH/${pre}nrm2.o $SRCPATH/${pre}ssq.o"  
      else
         $FKOPATH/bin/fko $iflag -o $SRCPATH/$pre${blas}.s \
            $FKOPATH/blas/$pre$blas.b
         #cerate obj using the assemblies
         $CC $CFLAGS -o $SRCPATH/$pre${blas}.o -c $SRCPATH/$pre${blas}.s
         OBJ="$OBJ $SRCPATH/$pre${blas}.o"
      fi
   done
   echo ""
#   
#     create library for desired prefix
#
   ar r $FKOLIBdir/${pre}${MLIB} $OBJ
   OBJ=
   echo ""
   ar r $FKOLIBdir/${pre}${NRM2LIB} $NRM2OBJ
   NRM2OBJ=
   echo ""
done

#######################################################
# now its time run and timer
########################################################

CMP=acml

cd $TIMEdir 
make clean

make xsl1blastst_ifko

`$TIMEdir/xsl1blastst_ifko "-C $C -n $N -X 3 1 1 1 -a 1 2.0 -R $R -F $F"` > $CMPPATH/sl1blas_${CMP}_ifko_$scripttime

make xdl1blastst_ifko

`$TIMEdir/xdl1blastst_ifko "-C $C -n $N -X 3 1 1 1 -a 1 2.0 -R $R -F $F"` > $CMPPATH/dl1blas_${CMP}_ifko_$scripttime

echo " SL1BLAS: $CMP vs. ifko"
echo " ======================="
cat $CMPPATH/sl1blas_${CMP}_ifko_$scripttime

echo " DL1BLAS: $CMP vs. ifko"
echo " ======================="
cat $CMPPATH/dl1blas_${CMP}_ifko_$scripttime

