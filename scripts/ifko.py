#! /usr/bin/python

import sys
import fkocmnd
import l1cmnd

opt = "-X 1 -Y 1 -Fx 16 -Fy 16"
#
#  Given a set of flags, try differing pf inst for read & write arrays
#
def ifko_pftype(ATLdir, ARCH, KF0, ncache, fko, rout, pre, blas, N,
                info, pfarrs, pfsets):
#
#  Time the default case
#
   fkocmnd.callfko(fko, KF0)
   [t0,m0] = l1cmnd.time(ATLdir, ARCH, pre, blas, N, "fkorout.s", 
                         "gcc", "-x assembler-with-cpp", opt=opt)
   assert(t0 > 0.0)
   print "base mf = %f.2" % m0
   KF = KF0
#
#  If there are output arrays, varying inst used for writes
#
   SETS = 0
   for i in pfsets:
      if i:
         SETS = 1
         break

   if SETS:
#
#     Try using 3DNow's prefetchw for all writes
#
      KF = KF0 + " -Paw 3"
      fkocmnd.callfko(fko, KF)
      [t1,m1] = l1cmnd.time(ATLdir, ARCH, pre, blas, N, "fkorout.s", 
                            "gcc", "-x assembler-with-cpp", opt=opt)
      if (m1 > 0.0):
         print "prefetchw speedup: %.3f !" %  (m1/m0)
#
#     Try temporal prefetch
#
      KF1 = KF0 + " -Paw 0"
      fkocmnd.callfko(fko, KF1)
      [t2,m2] = l1cmnd.time(ATLdir, ARCH, pre, blas, N, "fkorout.s", 
                            "gcc", "-x assembler-with-cpp", opt=opt)
      print "prefetchw speedup: %.3f !" %  (m2/m0)

      if m2 > m1:
         m1 = m2
         KF = KF1
      if m1 > m0:
         m0 = m1
         KF0 = KF
#
#  If we've got read-only arrays, try a varying pref inst for reads
#
   READO = 0
   for i in pfsets:
      if not i:
         READO = 1
         break
   if READO:
      KF = KF0 + " -Par 0"
      fkocmnd.callfko(fko, KF)
      [t1,m1] = l1cmnd.time(ATLdir, ARCH, pre, blas, N, "fkorout.s", 
                            "gcc", "-x assembler-with-cpp", opt=opt)
      print "prefetchr speedup: %.3f !" %  (m1/m0)
      if m1 > m0:
         KF0 = KF
         m0 = m1
       
#
#  For each array, find best cache level to fetch to
#
   NC = info[0]
   LS = info[1]
   for arr in pfarrs:
      j = 1
      while j < NC:
         KF = " -P %s %d %d" % (arr, j, LS[j])
         KF = KF0 + KF
         fkocmnd.callfko(fko, KF)
         [t1,m1] = l1cmnd.time(ATLdir, ARCH, pre, blas, N, "fkorout.s", 
                               "gcc", "-x assembler-with-cpp", opt=opt)
         print "lvl %d %s speedup: %.3f !" %  (j, arr, m1/m0)
         if m1 > m0:
            KF0 = KF
            m0 = m1
   return KF0

#
# attempt to find best prefetch distance for given array
#
def FindPFD(ATLdir, ARCH, KF0, fko, rout, pre, blas, N, info, arr,
            pfd0=0, pfdN=2048, pfdinc=0):
   st = "-P %s " % (arr)
   j = KF0.find(st)
   if j == -1:
      j = KF0.find("-P all ")
      assert(j != -1)
      j += 7
   else:
      j += 4 + arr.len()
   words = KF0[j:].split()
   pflvl = int(words[0])
   LS = info[1][pflvl];
   if not pfd0: pfd0 = LS
   if not pfdinc: pfdinc = LS
   print "\nFinding PFD for %s in [%d:%d:%d]" % (arr, pfd0, pfdN, pfdinc)
   ipd = int(words[1])
   pfd = pfd0
   mfM = 0.0
   pfdM = 0
   KF0 = KF0 + " -Ps b A 0 1"
   while pfd <= pfdN:
      KFn = KF0 + " -P %s %d %d" % (arr, pflvl, pfd)
      fkocmnd.callfko(fko, KFn)
      [t,mf] = l1cmnd.time(ATLdir, ARCH, pre, blas, N, "fkorout.s", 
                           "gcc", "-x assembler-with-cpp", opt=opt)
#      mfs.append(mf)
      print "   %s : PFD = %d mflop = %s" % (arr, pfd, mf)
      if mf > mfM:
         mfM = mf
         pfdM = pfd
      pfd += LS

   print "\nBEST prefetch distance = %d (%.2f)" % (pfdM, mfM)
   if (pfdM != ipd):
      KF0 = KF0 + " -P %s %d %d" % (arr, pflvl, pfdM)
   return (KF0)

def FindUR(ATLdir, ARCH, KF0, fko, rout, pre, blas, N, info, UR0=1, URN=128):

   print "Finding best unroll:"
#
#  Get rid of default unrolling so we can add our own
#
   j = KF0.find("-U ")
   if j != -1:
      words = KF0[j:].split()
      if (j != 0): KF0 = KF0[0:j]
      else : KF0 = ""
      for word in words[2:]:
         KF0 = KF0 + " "  + word
   UR = UR0
   mf0 = 0
   URB = 1
   while UR <= URN:
      KFn = KF0 + " -U %d" % (UR)
      fkocmnd.callfko(fko, KFn)
      [t,mf] = l1cmnd.time(ATLdir, ARCH, pre, blas, N, "fkorout.s", 
                           "gcc", "-x assembler-with-cpp", opt=opt)
      print "   UR=%d, mflop=%.2f" % (UR, mf)
#
#     Demand that higher unrollings get at least 1% better
      if (mf >= mf0*1.01):
         URB = UR
         mf0 = mf
      UR *= 2

   KF0 = KF0 + " -U %d" % URB
   return KF0

def ifko(l1bla, pre, N):
   (IFKOdir, fko) = fkocmnd.GetFKOinfo()
   (ATLdir, ARCH) = fkocmnd.FindAtlas(IFKOdir)
   rout =  IFKOdir + '/blas/' + pre + l1bla + '.b'
   outf =  ATLdir + '/tune/blas/level1/' + l1bla.upper() + '/fkorout.s'
   info = fkocmnd.info(fko, rout)
   ncache = info[0]
   (pfarrs, pfsets) = fkocmnd.GetPFInfo(info)
   npf = len(pfarrs)
   KFLAGS = fkocmnd.GetStandardFlags(fko, rout, pre)
   KFLAGS = KFLAGS + " -o " + str(outf) + " " + rout
   KFLAGS = ifko_pftype(ATLdir, ARCH, KFLAGS, ncache, fko, rout, pre, l1bla, N,
                        info, pfarrs, pfsets)
   print "\nFLAGS so far =", fkocmnd.RemoveFilesFromFlags(l1bla, KFLAGS)
#
#  Find best PFD for each pfarr
#
   for arr in pfarrs:
      KFLAGS = FindPFD(ATLdir, ARCH, KFLAGS, fko, rout, pre,l1bla, N, info, arr)
   KFLAGS = fkocmnd.RemoveRedundantPrefFlags(KFLAGS, pfarrs)
#
#  Find best unroll
#
   KFLAGS = FindUR(ATLdir, ARCH, KFLAGS, fko, rout, pre, l1bla, N, info)
   print "\nFLAGS so far =", fkocmnd.RemoveFilesFromFlags(l1bla, KFLAGS)

ifko(sys.argv[1], sys.argv[2], 80000)
