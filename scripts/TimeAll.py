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
psiz = [4, 8]
l1routs = ['asum', 'axpy', 'dot', 'scal', 'iamax']
l1refs  = ['asum_fabs1_x1.c', 'axpy1_x1y1.c', 'dot1_x1y1.c', 'scal1_x1.c',
           'iamax_abs1_x1.c']
l1atl   = []
CCatl   = []
CCFat   = []

#
# Find the cacheline size for default flag determination
#
bob = fkocmnd.info(fko, IFKOdir + "/blas/dasum.b")
NC = bob[0]
LS = bob[1][0]
VECT = bob[5]
print LS
VS = ""
KFLAG = "-P all 0 " + str(LS*2)

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
fkoT  = []
fkoMF = []
j = i = 0

CALLREF=1
CALLATL=1
CALLFKO=1
# print 'l1atl = ', l1atl
opt = "-X 1 -Y 1 -Fx 16 -Fy 16"
for blas in l1routs:
   for pre in pres:
      if (CALLREF != 0):
         [time,mf] = l1cmnd.time(ATLdir, ARCH, pre, blas, N, l1refs[i])
         print "REF %20.20s : time=%f, mflop=%f" % (pre+l1refs[i], time, mf)
         refT.append(time)
         refMF.append(mf)

      if (CALLATL != 0):
         [time,mf] = l1cmnd.time(ATLdir, ARCH, pre, blas, N, l1atl[j], 
                                 CCatl[j], CCFat[j])
         print "ATL %20.20s : time=%f, mflop=%f" % (pre+l1atl[j], time, mf)
         atlT.append(time)
         atlMF.append(mf)

      if (CALLFKO != 0):
         rout = IFKOdir + '/blas/' + pre + blas + '.b'
         outf = ATLdir + '/tune/blas/level1/' + blas.upper() + '/fkorout.s'
         info = fkocmnd.info(fko, rout)
         VEC = info[5]
         if (VEC == 0):
            UR = LS / psiz[j%2]
            KFLAGS = KFLAG + " "
         else:
            KFLAGS = KFLAG + " -V"
            UR = LS / 16
         KF0    = KFLAGS + " -U " + str(UR)
         KFLAGS = KF0 + ' -o ' + outf + ' ' + rout
         if (os.path.exists(outf)):
            os.remove(outf)
         fkocmnd.callfko(fko, KFLAGS)
         [time,mf] = l1cmnd.time(ATLdir, ARCH, pre, blas, N, 'fkorout.s',
                                 "gcc", "-x assembler-with-cpp", opt=opt)
         print "FKO %20.20s : time=%f, mflop=%f" % (pre+blas+'.b', time, mf)
         print "                           flags =", KF0
         fkoT.append(time)
         fkoMF.append(mf)
      j += 1
   i += 1

print r"OPERATION   & gcc+ref & icc+ref &gcc+atlas&icc+atlas& fko     &     ifko\\\hline\hline"
form = "%12s& %7.0f &         & %7.0f &         & %7.0f &        \\\\\\hline"
#form2= "%12s& %7.0f &         & %7.0f\\footnotemark[1] &         & %7.0f &        \\\\\\hline"
form2= "%12s& %7.0f &         & %7.0f*&         & %7.0f &        \\\\\\hline"

form3= "%12s& %7.2f &         & %7.2f &         & %7.2f &        \\\\\\hline"
form4= "%12s& %7.2f &         & %7.2f*&         & %7.2f &        \\\\\\hline"
#form4= "%12s& %7.2f &         & %7.2f\\footnotemark[1] &         & %7.2f &        \\\\\\hline"

#fkoMF = [1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1];
#atlMF = [1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1];
#refMF = [1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1];
print "\n\n"
i = 0
j = 0
for blas in l1routs:
   for pre in pres:
      if (CCFat[j] == None or CCFat[j].find("assembler") == -1):
         print form \
         % ('{\\tt ' + pre + blas + '}', refMF[j], atlMF[j], fkoMF[j])
      else:
         print form2 \
         % ('{\\tt ' + pre + blas + '}', refMF[j], atlMF[j], fkoMF[j])

      j += 1
   i += 1
i = 0
j = 0
print "\n\n"
for blas in l1routs:
   for pre in pres:
      if (CCFat[j] == None or CCFat[j].find("assembler") == -1):
         print form3 \
         % ('{\\tt ' + pre + blas + '}', 1.0, atlMF[j]/refMF[j], 
            fkoMF[j]/refMF[j])
      else:
         print form4 \
         % ('{\\tt ' + pre + blas + '}',  1.0, atlMF[j]/refMF[j], 
            fkoMF[j]/refMF[j])

      j += 1
   i += 1
