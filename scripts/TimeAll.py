#! /usr/bin/python
import os
import sys
import re
import l1cmnd

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


[ATLdir, ARCH] = FindAtlas(IFKOdir)
print ARCH
print "ATLdir='%s', ARCH='%s'" % (ATLdir, ARCH)

# [time,mflop] = l1cmnd.l1time(ATLdir, ARCH, 'd', 'dot', 80000, 'dot1_x1y1.c')
# print "time=%f, mflop=%f" % (time,mflop)

pres = ['s', 'd']
l1routs = ['asum', 'axpy', 'dot', 'scal']
l1refs  = ['asum_fabs1_x1.c', 'axpy1_x1y1.c', 'dot1_x1y1.c', 'scal1_x1.c']

for blas in l1routs:
   for pre in pres:
      [id,rout,CC, CCF] = l1cmnd.res(ATLdir, ARCH, blas, pre)
      print "%s%s_SUMM : ID=%d, rout=%s" % (pre, blas, id, rout)
      if (CC != None):
         print "   --> ucc='%s', ccflags='%s'" % (CC, CCF)
