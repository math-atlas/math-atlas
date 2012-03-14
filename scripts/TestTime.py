#! /usr/bin/python
import os
import sys
import l1cmnd
import fkocmnd

N = 80000
L = 1
TEST = 1
CC = "gcc"
#CCF = "-m32 -x assembler-with-cpp"
CCF = "-x assembler-with-cpp"
(IFKOdir,fko) = fkocmnd.GetFKOinfo()
[ATLdir,ARCH] = fkocmnd.FindAtlas(IFKOdir)
uopt=""

teopt = "-X 1 1 -Y 1 1 -Fx 16 -Fy 16"
#teopt = "-X 1 1 -Y 1 1 -Fx 32 -Fy 32"

nargs = len(sys.argv)
if nargs < 4:
   print 'USAGE: %s <pre> <blas> <file> [<N> <uopt> <TEST> <CC> <CCF>]' % sys.argv[0]
   sys.exit(1)

pre = sys.argv[1]
blas = sys.argv[2]
file = sys.argv[3]
if nargs > 4 :
   N = int(sys.argv[4])
   if nargs > 5 :
      TEST = int(sys.argv[5])
   if nargs > 6 :
      uopt = sys.argv[6]
      if nargs > 7 :
         CC = sys.argv[7]
         if nargs > 8 :
            CCF = sys.argv[8]

opt = "-X 1 -Y 1 -Fx 16 -Fy 16 " + uopt
#opt = "-X 1 -Y 1 -Fx 32 -Fy 32 " + uopt
[t0,mf] = l1cmnd.time(ATLdir, ARCH, pre, blas, N, file, cc=CC, ccf=CCF, opt=opt)
if TEST:
   i = l1cmnd.test(ATLdir, ARCH, pre, blas, N, file, cc=CC, ccf=CCF, opt=teopt)
   if i : PF = "FAIL"
   else : PF = "PASS"
else : PF = "SKIP"
print "%s%s-%s : time=%e, mflop=%.2f -- %s" % (pre, blas, file, t0, mf, PF)
