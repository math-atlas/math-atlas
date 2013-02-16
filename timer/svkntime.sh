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
         force=$OPTARG
         ;;
      s)
         skip=$OPTARG
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
#  seting parameter
#
   if [ -n "$skip" ] 
   then
      skip="--no "$skip
   fi

   if [ -n "$force" ] 
   then
      if [ "$force" == "s" ]
      then
         if [ -z "$skip" ]
         then
            skip="--no v"
            force=
         else
            skip=$skip",v"
            force=
         fi
      else
         force="--force "$force
      fi
   fi
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
#
#        specially handle the size for irkamax
#
         N1=
         if [ $kn = irk1amax ]
         then
            N1=$((N/2))
         elif [ $kn = irk2amax ]
         then
            N1=$((8*(N/24)))  # N/3
         elif [ $kn = irk3amax ]
         then
            N1=$((8*(N/32)))   # N/4
         else
            N1=$N
         fi
         echo "$SCRIPTdir/ifko.py $kn $pre $N1 $force $skip > $inputlog"
         $SCRIPTdir/ifko.py $kn $pre $N1 $force $skip > $inputlog
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
   path=
   if [ -z "$force" ]
   then
      echo "must speicfy option for untuned like: SV, S, VRC, VMMR "
      exit 1
   else
      if [ "$force" == "sv" ]
      then
         force="-V"
      elif [ "$force" == "vrc" ]
      then
         force="-rc -V"
      elif [ "$force" == "vmmr" ]
      then
         force="-mmr -V"
      elif [ "$force" == "s" ]
      then
         force=""
      else
         echo "set appropriate value for -f "
         exit 1
      fi
   fi

   for pre in $precision
   do
      echo "Untuned kernel for precision =" ${pre} " using $force ... ..."
      echo "============================================================="

      for kn in $kernel
      do
#
#        set path for sv
#
         if [ "$force" == "-V" ]
         then
            if [ $kn = sin ]
            then
               path=" -p 2"
            else
               path=" -p 1"
            fi
         else
            path=""
         fi
#
#        set appropriate N
#
         N1=
         if [ $kn = irk1amax ]
         then
            N1=$((N/2))
         elif [ $kn = irk2amax ]
         then
            N1=$((8*(N/24)))  # N/3
         elif [ $kn = irk3amax ]
         then
            N1=$((8*(N/32)))   # N/4
         else
            N1=$N
         fi

         inputlog=$LOGPATH/$pre${kn}_${N}_${scripttime}
         #echo 'make $pre$kn $N1 KFLAGS="'$force$path'" '
         cd $SCRIPTdir
         make $pre$kn $N1 KFLAGS="$force$path" > $inputlog 2> /dev/null
         #lflag=`tail -n 2 $inputlog | head -n 1`
         lflag=`tail -n 1 $inputlog`
         #pref=`echo $lflag | cut -d' ' -f 4 -s`
         #echo $pre$kn ":" $perf >> $RESULTPATH/result_$scripttime
         echo $lflag 
      done
   done
   #cat $RESULTPATH/result_$scripttime

fi





