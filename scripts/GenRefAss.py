#! /usr/bin/python
import os
import sys
import re
import l1cmnd
import fkocmnd

def GenAss(ATLdir, ARCH, OUTdir, pre, blas, N, rout, cc=None, ccf=None, opt=""):
#
#  Extract the original seconds & summation time & mflop
#
   l1sec = re.compile(r"tim=(.*)\s.*$")
   l1sum = re.compile(r"N=(.*),\s.*time=(.*), mflop=(.*)\s.*$")
   PROFILE = 1

   if (opt != ""):
      opt = 'opt="' + opt + '"'
   if cc != None and cc.find("iccprof") != -1: PROFILE = 1
   else : PROFILE = 0
      
   outf = ' -S -o ' + OUTdir + '/' + pre + blas + '.s'
   if PROFILE:
      popt = opt + ' UCC=icc UCCFLAGS="' + ccf + ' -prof_genx -prof_dir /tmp"'
      pcmnd = 'rm -f /tmp/*.dyn ; cd %s/tune/blas/level1/%s ; make %s%scase N=%d urout=%s %s' % \
             (ATLdir, ARCH, pre, blas, N, rout, popt)
#      print "cmnd = '%s'" % (pcmnd)
      fo = os.popen(pcmnd, 'r')
      lines = fo.readlines()
      err = fo.close()
      if err : print lines
      assert(err == None)
      zopt = opt + ' UCC=icc UCCFLAGS="' + ccf + ' -prof_use -prof_dir /tmp ' \
             + outf + '"'
   if PROFILE:
      opt = zopt
   elif cc != None :
      opt = opt + ' UCC=' + cc + ' UCCFLAGS="' + ccf + outf + '"'
   else:
      opt = opt + ' UCCFLAGS="' + outf + '"'
#   print "opt = '%s'" % opt
   cmnd = 'make %s%scase N=%d urout=%s %s' % (pre, blas, N, rout, opt)
   cmnds = 'cd %s/tune/blas/level1/%s ; %s ' % (ATLdir, ARCH, cmnd)
   print "cmnds = '%s'" % (cmnds)
   fo = os.popen(cmnds, 'r')
   lines = fo.readlines()
   err = fo.close()

(IFKOdir,fko) = fkocmnd.GetFKOinfo()
OUTdir = IFKOdir + '/blas/assembly'


(ATLdir, ARCH) = fkocmnd.FindAtlas(IFKOdir)
print ARCH
print "ATLdir='%s', ARCH='%s'" % (ATLdir, ARCH)

# [time,mflop] = l1cmnd.l1time(ATLdir, ARCH, 'd', 'dot', 80000, 'dot1_x1y1.c')
# print "time=%f, mflop=%f" % (time,mflop)

#
# Defaults
#
N=80000
pres = l1cmnd.GetDefaultPre()
l1routs = l1cmnd.GetDefaultBlas()
uopt = ""
#
# User overrides
#
nargs = len(sys.argv)
if (nargs > 1):
   blas = sys.argv[1]
   if blas.find("default") == -1:
      words = blas.split(",")
      l1routs = []
      for word in words:
         l1routs.append(word)

   if (nargs > 2):
      pre = sys.argv[2]
      if pre.find("default") == -1:
         words = pre.split(",")
         pres = []
         for word in words:
            pres.append(word)

      if nargs > 3:
         N = int(sys.argv[3])
         if nargs > 4:
            uopt = sys.argv[4]
l1refs  = l1cmnd.GetDefaultRefBlas(l1routs)

opt = "-X 1 -Y 1 -Fx 16 -Fy 16 " + uopt
 
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
print "LINESIZE = %d" % LS
VS = ""
KFLAG = "-P all 0 " + str(LS*2)


refT  = []
refMF = []
atlT  = []
atlMF = []
fkoT  = []
fkoMF = []
j = i = 0

CALLREF=1
PROFILE=0
if PROFILE:
   PROFCC="iccprof"
   PROFF = "-xP -O3 -mp1 -static -w"
else:
   PROFCC=None
   PROFF=None
#PROFCC = 
# print 'l1atl = ', l1atl
for blas in l1routs:
   for pre in pres:
      GenAss(ATLdir, ARCH, OUTdir, pre, blas, N, l1refs[i], 
             opt=opt, cc=PROFCC, ccf=PROFF)
      print "Generated " + OUTdir + '/' + pre + blas + '.s'
      j += 1
   i += 1
