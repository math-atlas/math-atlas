#!/bin/bash

#
#  default values for the global variable
#
ATLver=3.11.9
OBJdir=Linux_X8664_AVX
ATLASdir=/home/msujon/ATLAS
kernels="nrm2 amax sin cos irk1amax irk2amax irk3amax"
ker=

usage="Usage: $0 [OPTION]... -key key_value

Options:
-v       specify version of ATLAS
-o       directory of obj dir in atlas
-k       name of sinlge kernel to setup inside atlas, default all
--help   display help and exit
"

#
#  parsing arguments using getopts tool 
#

while getopts "v:o:k:" opt 
do
   case $opt in 
      v)
         ATLver=$OPTARG
         ;;
      o) 
         OBJdir=$OPTARG
         ;;
      k)
         ker=$OPTARG
         ;;
      \?)
         echo "$usage"
         exit 1
         ;;
      esac
done

#
#  update the the ATLAS dir 
#
ATLAS_ROOT=${ATLASdir}/ATLAS$ATLver
ATLAS_OBJ=${ATLAS_ROOT}/$OBJdir

if [ ! -e $ker ];
then
   #echo $ker
   kernels=$ker
fi


#echo $ATLAS_ROOT
#echo $ATLAS_OBJ
#echo $kernels

srcdir=

for kn in ${kernels}
do
   if [ ! $kn = nrm2 ]
   then
      echo "create dir and links for $kn..."
      dir=`echo $kn | tr '[a-z]' '[A-Z]'`  
      srcdir=$ATLAS_ROOT/tune/blas/level1/$dir
      mkdir $srcdir
      ln -s $srcdir $ATLAS_OBJ/tune/blas/level1/$dir
      #echo "mkdir $srcdir"
      #echo "ln -s $srcdir $ATLAS_OBJ/tune/blas/level1/$dir"
   fi

   if [ $kn = amax ]
   then
      echo "copying stub file for $kn"
      cp amax_stub_test.c $srcdir 
      cp amax_stub_time.c $srcdir 
      #echo "cp amax_stub_test.c $srcdir "
      #echo "cp amax_stub_time.c $srcdir "
   else
      echo "coping test and time file for $kn ... ..."
      cp ${kn}test.c $ATLAS_ROOT/tune/blas/level1/
      cp ${kn}time.c $ATLAS_ROOT/tune/blas/level1/
      #echo "cp ${kn}test.c $ATLAS_ROOT/tune/blas/level1/"
      #echo "cp ${kn}time.c $ATLAS_ROOT/tune/blas/level1/"
   fi
done

echo "copying extension of the Makefile..."
cp Make.ifko $ATLAS_OBJ/tune/blas/level1/
#echo "cp Make.ifko $ATLAS_OBJ/tune/blas/level1/"
echo "Right now manually update the Makefile by including this make.ifko"




