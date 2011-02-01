#!/bin/sh
# USAGE: ./scope.sh <arch>
echo ================ CBLAS ==================
fgrep -e fault -e FAULT -e error -e ERROR -e fail -e FAIL res/C_BLAS* | \
      fgrep -v PASSED
echo ================ F77BLAS ==================
fgrep -e fault -e FAULT -e error -e ERROR -e fail -e FAIL res/F77_BLAS* | \
      fgrep -v PASSED
echo ================== BIN ====================
fgrep -e fault -e FAULT -e error -e ERROR -e fail -e FAIL res/BIN_SUMM
echo ================= PTBIN ===================
fgrep -e fault -e FAULT -e error -e ERROR -e fail -e FAIL res/BIN_PTSUMM
echo ================= DONE  ===================

