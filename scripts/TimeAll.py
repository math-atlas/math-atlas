#! /usr/bin/python
import os
import sys
import re
import l1cmnd
import fkocmnd

def FindAtlas(FKOdir):
   file = os.path.join(FKOdir, 'time')
   file = os.path.join(file, 'Makefile')

   fi = open(file, 'r')
   for line in fi.readlines():
      if (line.startswith('include')):
         j = line.find('Make.')
         assert(j != -1)
         ARCH = line[j+5:].strip()
         ATLdir = line[8:j-1].strip()
         break
   else:
      print "Can't find include line in %s" % path+Makefile
      sys.exit(-1);
   fi.close()

   return [ATLdir, ARCH]

pwd = os.getcwd()
j = pwd.rfind('iFKO')
# IFKOdir = '/home/rwhaley/PROJ/iFKO'
IFKOdir = pwd[0:j+4]
fko = IFKOdir + '/bin/fko'


[ATLdir, ARCH] = FindAtlas(IFKOdir)
print ARCH
print "ATLdir='%s', ARCH='%s'" % (ATLdir, ARCH)

# [time,mflop] = l1cmnd.l1time(ATLdir, ARCH, 'd', 'dot', 80000, 'dot1_x1y1.c')
# print "time=%f, mflop=%f" % (time,mflop)

pres = ['s', 'd']
l1routs = ['asum', 'axpy', 'dot', 'scal']
l1refs  = ['asum_fabs1_x1.c', 'axpy1_x1y1.c', 'dot1_x1y1.c', 'scal1_x1.c']
l1atl   = []
CCatl   = []
CCFat   = []

#
# Find the cacheline size for default flag determination
#
bob = fkocmnd.info(fko, IFKOdir + "/blas/dasum.b")
LS = bob[1][0]
print LS
sys.exit(0)
N=80000

#
# Find the kernel names and compiler flags used by ATLAS on this arch
#
for blas in l1routs:
   for pre in pres:
      [id,rout,CC, CCF] = l1cmnd.res(ATLdir, ARCH, blas, pre)
      l1atl.append(rout)
      CCatl.append(CC)
      CCFat.append(CCF)
#      print "%s%s_SUMM : ID=%d, rout=%s" % (pre, blas, id, rout)
#      if (CC != None):
#         print "   --> ucc='%s', ccflags='%s'" % (CC, CCF)

refT  = []
refMF = []
atlT  = []
atlMF = []
j = i = 0
print 'l1atl = ', l1atl
for blas in l1routs:
   for pre in pres:
      [time,mf] = l1cmnd.time(ATLdir, ARCH, pre, blas, N, l1refs[i])
      print "REF %20.20s : time=%f, mflop=%f" % (pre+l1refs[i], time, mf)
      refT.append(time)
      refMF.append(mf)
      [time,mf] = l1cmnd.time(ATLdir, ARCH, pre, blas, N, l1atl[j], 
                              CCatl[j], CCFat[j])
      print "ATL %20.20s : time=%f, mflop=%f" % (pre+l1atl[j], time, mf)
      atlT.append(time)
      atlMF.append(mf)
      j += 1
   i += 1
