#! /usr/bin/python

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
   print "FLAGS so far =", KFLAGS

ifko("axpy", "s", 80000)
