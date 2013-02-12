#!/bin/bash
#
# save the runtime, it will use as id in every logfiles
#
scripttime=`date +%Y%m%d_%H%M%S`
echo "ID :" $scripttime
#
# set default parameter 
#
#FKOPATH=/home/msujon/iFKO
FKOPATH=/home/msujon/Research/working/iFKO-SV/iFKO

kernel="amax iamax nrm2 asum sin cos irk1amax irk2amax irk3amax"
rckernel="amax asum sin cos"
tuned="tuned"
force=
skip=
del=0
precision="s d"
N=16000 

#
#  commandline argument
#
usage="Usage: $0 [OPTION]... 
Options:
-p       precision of kernel, s for single and d for double precision
-k       name of sinlge kernel, default is all
-o       type of optimization: tuned, untuned. default is tuned
-f       force to any tranformation in tuner like: SV, default none
-s       skip to any optimization in tuner like: V, default none
-n       number of elements to time
-d       takes no argument. delete all log files and paths, use with caution!
--help   display help and exit
"

#
#  parsing arguments using getopts tool 
#  option with argument must postfixed by : 
#  so, d will terminate with any colon ':'
#

while getopts "p:k:t:f:s:n:d" opt 
do
   case $opt in 
      p)
         precision=$OPTARG
         ;;
      k) 
         kernel=$OPTARG
         ;;
      t)
         tuned=$OPTARG
         ;;
      f)
         force="--force "$OPTARG
         ;;
      s)
         skip="--skip "$OPTARG
         ;;
      n)
         N=$OPTARG
         ;;
      d)
         del=1
         ;;
      \?)
         echo "$usage"
         exit 1
         ;;
      esac
done

#
#  want to delete all the results and log files!
#
if [ $del -eq 1 ]
then
   echo -n "Are you sure to delete all logfiles [y/n]: "
   read -e del
   if [ $del == "y" ]
   then
      echo "deleting all logfiles"
      rm -r -f ${FKOPATH}/timer/result
      rm -r -f ${FKOPATH}/timer/log
      exit 0
   else
      echo "exiting without deleting"
      exit 1
   fi
fi


#
#  set all paths for logs
#

TIMEdir=${FKOPATH}/timer
SCRIPTdir=${FKOPATH}/scripts

CMPPATH=$TIMEdir/result
mkdir -p $CMPPATH

LOGdir=$TIMEdir/log
mkdir -p $LOGdir
#
# create log, result and src dir
#
for dir in svkn_src svkn_result svkn_log
do
   mkdir -p $LOGdir/$dir
   mkdir -p $LOGdir/$dir/$scripttime
done

RESULTPATH=$LOGdir/svkn_result
SRCPATH=$LOGdir/svkn_src/$scripttime
LOGPATH=$LOGdir/svkn_log/$scripttime

#
#  commenting all the prints
#
if [ "0" == "1" ]
then
   echo "FKO Path: " $FKOPATH
   echo "Result Path: " $RESULTPATH
   echo "Source Path: " $SRCPATH
   echo "Library Path: " $FKOLIBdir
   echo "Logfiles Path: " $LOGPATH
   echo "Comparison-result Path: " $CMPPATH
   echo ""
fi

if [ "0" == "1" ] 
then
   echo "FKO Path: " $FKOPATH
   echo "Kernel = "$kernel
   echo "pre = "$pre
   echo "opt =" $tuned 
   echo "force = "$force
   echo "skip = "$skip
fi

#
# #############################################################################
#

if [ "$tuned" == "tuned" ] 
then
#
#  applying tunning 
#
   for pre in $precision
   do
      echo "Tuning kernel for precision =" ${pre} "... ... ..."
      echo "=================================================="
      for kn in $kernel
      do
         inputlog=$LOGPATH/$pre${kn}_${N}_${scripttime}
         $SCRIPTdir/ifko.py $kn $pre $N $force $skip> $inputlog
         lflag=`tail -n 8 $inputlog | head -n 1`
         iflag=`echo $lflag | cut -d' ' -f 6- -s`
         fline=`tail -n 5 $inputlog | head -n 1`
         perf=`echo $fline | cut -d' ' -f 4 -s`
         echo $pre${kn}": "${perf}" ["${iflag}"]" 
         echo $pre$kn ":" $perf >> $RESULTPATH/result_$scripttime
         echo "     " $iflag >> $RESULTPATH/result_$scripttime
      done
   done
else
#
# not tuned, apply without any tuning
#
   echo "not tuned selected"
fi





