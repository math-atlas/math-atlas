#/bin/bash

SPATH=/home/msujon/Research/working/iFKO-SV/timer

#
#  speculative tuned and untuned 
#
$SPATH/svkntime.sh -f sv | tee sv-res-tuned.txt
$SPATH/svkntime.sh -f sv -t u | tee sv-res-untuned.txt

#
#  serial
#
$SPATH/svkntime.sh -f s | tee serial-res-tuned.txt
$SPATH/svktime.sh -f s -s rc,mmr | tee serial-noX-res.txt

$SPATH/svkntime.sh -f s -t u | tee serial-res-untuned.txt

#
# RC 
#
$SPATH/svkntime.sh -f vrc | tee vrc-res-tuned.txt
$SPATH/svkntime.sh -f vrc -t u | tee vrc-res-tuned.txt


