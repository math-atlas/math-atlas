#! /usr/bin/python -tt

import sys
import fkocmnd
import cmnd
import kernels
import re

#
# Global Variable to save the Altas opt flag  
# optT is for tester and opt is for timer
#

#optT = "-X 1 1 -Y 1 1 -Fx 32 -Fy 32"
optT = "-Fx 32 -Fy 32 -Fa 32"

opt = "" ## populated with user argument 
skipOpt = [] ## opt named as(fko cmnd): mmr,rc,v,sv,se,P,ps,par,p
forceOpt = [] ## sv, vrc, vmmx : only used to force a vector method now 

SB = 0 # temporary global just to test
URF = 0 # forced UR, tuned with fixed UR
isSV = 0 # special flag for speculation applied!

#
# Given set of arrs that are write-only (no uses), tries using non-temporal
# writes
#
def ifko_writeNT(ATLdir, ARCH, KF0, fko, rout, pre, l1bla, N, M, lda, wnt):
#
#  Time the default case
#
   warrs = []
   fkocmnd.callfko(fko, KF0)
   [t0,m0] = cmnd.time(ATLdir, ARCH, pre, l1bla, N, M, lda, "fkorout.s", 
                         "gcc", "-x assembler-with-cpp", opt=opt)
   print "WNT none : %.2f" % (m0)
   for wa in wnt:
      KFN = KF0 + " -W " + wa
      fkocmnd.callfko(fko, KFN)
      [tN,mN] = cmnd.time(ATLdir, ARCH, pre, l1bla, N, M,lda, "fkorout.s", 
                            "gcc", "-x assembler-with-cpp", opt=opt)
      print "WNT %s : %2.f" % (wa, mN)
      if mN > m0:
         KF0 = KFN
         m0 = mN
         warrs.append(wa)
      
   return [m0,KF0,warrs]
#
#  Given a set of flags, try differing pf inst for read & write arrays
#
def ifko_pftype(ATLdir, ARCH, KF0, ncache, fko, rout, pre, blas, N, M, lda, 
                info, pfarrs, pfsets):
#
#  Time the default case
#
   fkocmnd.callfko(fko, KF0)
   [t0,m0] = cmnd.time(ATLdir, ARCH, pre, blas, N, M,lda, "fkorout.s", 
                         "gcc", "-x assembler-with-cpp", opt=opt)
   assert(t0 > 0.0)
   print "   base mf = %f.2" % m0
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
      [t1,m1] = cmnd.time(ATLdir, ARCH, pre, blas, N, M, lda, "fkorout.s", 
                            "gcc", "-x assembler-with-cpp", opt=opt)
      if (m1 > 0.0):
         print "   prefetchw speedup: %.3f !" %  (m1/m0)
#
#     Try temporal prefetch
#
      KF1 = KF0 + " -Paw 0"
      fkocmnd.callfko(fko, KF1)
      [t2,m2] = cmnd.time(ATLdir, ARCH, pre, blas, N, M, lda, "fkorout.s", 
                            "gcc", "-x assembler-with-cpp", opt=opt)
      print "   prefetchw speedup: %.3f !" %  (m2/m0)

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
      [t1,m1] = cmnd.time(ATLdir, ARCH, pre, blas, N, M, lda, "fkorout.s", 
                            "gcc", "-x assembler-with-cpp", opt=opt)
      print "   prefetchr speedup: %.3f !" %  (m1/m0)
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
         [t1,m1] = cmnd.time(ATLdir, ARCH, pre, blas, N, M, lda, "fkorout.s", 
                               "gcc", "-x assembler-with-cpp", opt=opt)
         print "   lvl %d %s speedup: %.3f !" %  (j, arr, m1/m0)
         if m1 > m0:
            KF0 = KF
            m0 = m1
         j += 1
   return [m0,KF0]

#
# attempt to find best prefetch distance for given array
#
def FindPFD(ATLdir, ARCH, KF0, fko, rout, pre, blas, N, M, lda, info, arr,
            pfd0=0, pfdN=2048, pfdinc=0):
#
#  Figure out the prefetch cache level
#
   st = "-P %s " % (arr)
   j = KF0.find(st)

   if j == -1:
      j = KF0.find("-P all ")
      assert(j != -1)
      j += 7
   else:
      j += 4 + len(arr)
   words = KF0[j:].split()
   pflvl = int(words[0])
   
   LS = info[1][pflvl];
   if not pfd0: pfd0 = LS
   if not pfdinc: pfdinc = LS
   print "\n   Finding PFD for %s in [%d:%d:%d]" % (arr, pfd0, pfdN, pfdinc)
   ipd = int(words[1])
   mfM = 0.0
   pfdM = 0
#
#  Majedul: delete previous default pref sch
# 
#   match = re.search(r'-Ps\s\w\s\w\s\d+\s\d+',KF0) ## find -Ps b A 0 1
#   if match:
#      pat = match.group()
#      KF0 = KF0.replace(pat, '') ## remove the all occurance
#   KF0 = KF0 + " -Ps b A 0 1"
#
#  will delete the -P all 0 128
#
#   match = re.search(r'-P\s\w+\s[-]?\d+\s\d+',KF0) ## find -P X 0 1
#   if match:
#      pat = match.group()
#      KF0 = KF0.replace(pat, '') ## remove all occurance

#
#  Scope very short PFD
#
   if pfd0 >= LS and LS > 32:
      pfd = 32
      while pfd <= LS:
         KFn = KF0 + " -P %s %d %d" % (arr, pflvl, pfd)
         fkocmnd.callfko(fko, KFn)
         [t,mf] = cmnd.time(ATLdir, ARCH, pre, blas, N, M, lda, "fkorout.s", 
                              "gcc", "-x assembler-with-cpp", opt=opt)
         print "      %s : PFD = %d mflop = %.2f" % (arr, pfd, mf)
         if mf > mfM*1.0001:
            mfM = mf
            pfdM = pfd
         pfd += 8
      
   pfd = pfd0
   while pfd <= pfdN:
      KFn = KF0 + " -P %s %d %d" % (arr, pflvl, pfd)
      fkocmnd.callfko(fko, KFn)
      [t,mf] = cmnd.time(ATLdir, ARCH, pre, blas, N, M, lda, "fkorout.s", 
                           "gcc", "-x assembler-with-cpp", opt=opt)
#      mfs.append(mf)
      print "      %s : PFD = %d mflop = %.2f" % (arr, pfd, mf)
#
#     Demand higher PFD is at least 1.0001 faster
#
      if mf > mfM*1.0001:
         mfM = mf
         pfdM = pfd
      pfd += LS
   print "\n   BEST prefetch distance = %d (%.2f)" % (pfdM, mfM)
#
#  Try not prefetching array at all
#
   KFn = KF0 + " -P %s -1 0" % (arr)
#   print KFn
   fkocmnd.callfko(fko, KFn)
   [t,mf] = cmnd.time(ATLdir, ARCH, pre, blas, N, M, lda, "fkorout.s", 
                        "gcc", "-x assembler-with-cpp", opt=opt)
   print "      %s : NO PREFETCH:   mflop = %.2f" % (arr, mf)
   if mf >= mfM:
      KF0 = KF0 + " -P %s -1 0" % (arr)
      mfM = mf
   elif pfdM != ipd:
      KF0 = KF0 + " -P %s %d %d" % (arr, pflvl, pfdM)
   return [mfM,KF0]

def FindUR(ATLdir, ARCH, KF0, fko, rout, pre, blas, N, M, lda, info, UR0=1, URN=64):

   print "   Finding best unroll:"
#
#  Get rid of default unrolling so we can add our own

#
#  default max unroll, URN is 64
#  but for now, we consider 32 for cos 
#
   if blas.find("cos") != -1 :
      URN = 32;
#
#  if speculation is applied, max_unroll is applied to 16
# 
   if isSV:
      if blas.find("sin") != -1 : 
         URN = 8;
      elif  blas.find("cos") != -1 :
         URN = 5;  # FIXME: bitvec exceeds the datatype limit
      else:
         URN = 16
#
#     if SB is specified, SB*UR can be MaxUnroll
#
      if SB:
         URN = int (URN+SB-1)/SB 

#  MAX_UR = 32
#   if blas.find("amax") != -1 :
#      if SB:
#         URN = int((MAX_UR+SB-1)/SB) ### just testing!!
#      else:
#         URN = MAX_UR
#   if blas.find("iamax") != -1 :
#      if SB:
#         URN = int((MAX_UR+SB-1)/SB)
#      else:
#         URN = MAX_UR
#   if blas.find("nrm2") != -1 : ## limit the blind unrolling for nrm2... 
#      if SB:
#         URN = int((MAX_UR+SB-1)/SB)
#      else:
#         URN = MAX_UR
   
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
      [t,mf] = cmnd.time(ATLdir, ARCH, pre, blas, N, M,lda, "fkorout.s", 
                           "gcc", "-x assembler-with-cpp", opt=opt)
      print "      UR=%d, mflop=%.2f" % (UR, mf)
#
#     Demand that higher unrollings get at least 1% better
#
      if (mf >= mf0*1.01):
         URB = UR
         mf0 = mf
#
#     for SV, we need to increament by 1 
#
      if isSV:
         UR += 1
      else:
         UR *= 2
         #UR += 1

   KF0 = KF0 + " -U %d" % URB
   print "\n   BEST Unroll Factor = %d" %URB
   return [mf0,KF0]

def FindSE(ATLdir, ARCH, KF0, fko, rout, pre, blas, N, M, lda, acc, maxlen=6):
   """ Exactly same as FindAE but the fko command is changed 
   """
#
#  Time the default case
#  FIXED: need to save the default flag also. If SE is no better than previous
#  one, restore the default flag
#
   fkocmnd.callfko(fko, KF0)
   [t0,m0] = cmnd.time(ATLdir, ARCH, pre, blas, N, M,lda, "fkorout.s", 
                         "gcc", "-x assembler-with-cpp", opt=opt)
   mf0 = m0
#
#  Find present unrolling factor, and remove it from flags
#
   j = KF0.find("-U ")
   assert(j != -1)
   words = KF0[j:].split()
   ur = int(words[1])
   if j > 0: KFN = KF0[0:j-1]
   else : KFN = ""
   for word in words[2:] :
      KFN = KFN + " " + word
   #KF0 = KFN ## don't! lost the previous -U 
   
   print "   Finding ScalarExpan, UR=%d, mflop= %.2f" % (ur, m0)
   mfB = 0.0
   urB = ur
   aeB = 1
   nacc = len(acc)
   for ac in acc:
      i = 2
      while i <= maxlen:
         if ur >= i :
            KFLAG = KFN + " -U %d -SE %s %d" % (ur, ac, i)
            fkocmnd.callfko(fko, KFLAG)
            [t,mf] = cmnd.time(ATLdir, ARCH, pre, blas, N, M, lda, "fkorout.s", 
                                 "gcc", "-x assembler-with-cpp", opt=opt)
            print "      '%s' SE=%d, UR=%d, mflop= %.2f" % (ac, i, ur, mf)
            if mf > mfB:
               mfB = mf
               urB = ur
               aeB = i
         if i < ur or ur%i :
            j = ((ur+i-1) / i)*i
            if j != ur:
               KFLAG = KFN + " -U %d -SE %s %d" % (j, ac, i)
               fkocmnd.callfko(fko, KFLAG)
               [t,mf] = cmnd.time(ATLdir, ARCH, pre, blas, N, M, lda, "fkorout.s", 
                                    "gcc", "-x assembler-with-cpp", opt=opt)
               print "      '%s' SE=%d, UR=%d, mflop= %.2f" % (ac, i, j, mf)
               if mf > mfB:
                  mfB = mf
                  urB = j
                  aeB = i

            j = (ur / i) * i
            if j  and j != ur:
               KFLAG = KFN + " -U %d -SE %s %d" % (j, ac, i)
               fkocmnd.callfko(fko, KFLAG)
               [t,mf] = cmnd.time(ATLdir, ARCH, pre, blas, N,M,lda, "fkorout.s", 
                                    "gcc", "-x assembler-with-cpp", opt=opt)
               print "      '%s' SE=%d, UR=%d, mflop= %.2f" % (ac, i, j, mf)
            if mf > mfB:
               mfB = mf
               urB = j
               aeB = i
         i += 1

      if mfB > mf0*1.001:
         KFN = KFN + " -U %d -SE %s %d" % (urB, ac, aeB)
         mf0 = mfB
#
#  check for altimate result
#
   #print mfB, KFN
   #print mf0, KF0

   if mfB > m0*1.001:
      print "   SE=%d, UR=%d, mfB=%2.f, KFN=%s" % (aeB, urB, mfB, KFN)
   else:
      KFN = KF0         ## restore original flags
      mfB = m0
      print "   KFN = KF0 = %s" % KFN
   return[mfB, KFN]

def FindAE(ATLdir, ARCH, KF0, fko, rout, pre, blas, N, M, lda, acc, maxlen=6):
#
#  Time the default case
#
   fkocmnd.callfko(fko, KF0)
   [t0,m0] = cmnd.time(ATLdir, ARCH, pre, blas, N, M, lda, "fkorout.s", 
                         "gcc", "-x assembler-with-cpp", opt=opt)
   mf0 = m0
#
#  Find present unrolling factor, and remove it from flags
#
   j = KF0.find("-U ")
   assert(j != -1)
   words = KF0[j:].split()
   ur = int(words[1])
   if j > 0: KFN = KF0[0:j-1]
   else : KFN = ""
   for word in words[2:] :
      KFN = KFN + " " + word
   KF0 = KFN
   print "   Finding AccumExpan, UR=%d, mflop= %.2f" % (ur, m0)
   mfB = 0.0
   urB = ur
   aeB = 1
   nacc = len(acc)
   for ac in acc:
      i = 2
      while i <= maxlen:
         if ur >= i :
            KFLAG = KFN + " -U %d -AE %s %d" % (ur, ac, i)
            fkocmnd.callfko(fko, KFLAG)
            [t,mf] = cmnd.time(ATLdir, ARCH, pre, blas, N, M, lda, "fkorout.s", 
                                 "gcc", "-x assembler-with-cpp", opt=opt)
            print "      '%s' AE=%d, UR=%d, mflop= %.2f" % (ac, i, ur, mf)
            if mf > mfB:
               mfB = mf
               urB = ur
               aeB = i
         if i < ur or ur%i :
            j = ((ur+i-1) / i)*i
            if j != ur:
               KFLAG = KFN + " -U %d -AE %s %d" % (j, ac, i)
               fkocmnd.callfko(fko, KFLAG)
               [t,mf] = cmnd.time(ATLdir, ARCH, pre, blas, N, M, lda, "fkorout.s", 
                                    "gcc", "-x assembler-with-cpp", opt=opt)
               print "      '%s' AE=%d, UR=%d, mflop= %.2f" % (ac, i, j, mf)
               if mf > mfB:
                  mfB = mf
                  urB = j
                  aeB = i

            j = (ur / i) * i
            if j  and j != ur:
               KFLAG = KFN + " -U %d -AE %s %d" % (j, ac, i)
               fkocmnd.callfko(fko, KFLAG)
               [t,mf] = cmnd.time(ATLdir, ARCH, pre, blas, N, M, lda, "fkorout.s", 
                                    "gcc", "-x assembler-with-cpp", opt=opt)
               print "      '%s' AE=%d, UR=%d, mflop= %.2f" % (ac, i, j, mf)
            if mf > mfB:
               mfB = mf
               urB = j
               aeB = i
         i += 1

      if mfB > mf0*1.001:
         KFN = KFN + " -U %d -AE %s %d" % (urB, ac, aeB)
         mf0 = mfB

   print "   AE=%d, UR=%d, mfB=%2.f, KFN=%s" % (aeB, urB, mfB, KFN)
   return[mfB, KFN]

#
# Returns array of indexes in st0 that are also in st1
#
def FindMatchList(st0, st1):
   n0 = len(st0)
   n1 = len(st1)
   i = 0
   matches = []
   while i < n0:
      j = 0
      while j < n1:
         if st0[i] == st1[j] or \
            (len(st0[i]) == len(st1[j]) and st0[i].find(st1[j]) != -1):
            matches.append(i)
         j += 1
      i += 1
   return(matches)


def ifko_PathXform(ATLdir, ARCH, KF0, ncache, fko, rout, pre, blas, N, M, lda,
                   npath, red1p):
   """ This function will apply some basic transformation on scalar code 
       like: fall-thru xform, rc, mmr etc
       should only call this when npath > 1
   """
#
#  checkout the default performance 
#
   #print KF0 
   fkocmnd.callfko(fko, KF0)
   [t0,m0] = cmnd.time(ATLdir, ARCH, pre, blas, N, M, lda, "fkorout.s", 
                         "gcc", "-x assembler-with-cpp", opt=opt)
#
#  must have multiple paths 
#  
   if npath == 1 :
      print 'Single path! No Xform can be applied!!'
      sys.exit(1);
   
#
#  apply fall-thru optimization and see which fall-thru path is better
#
   KFn = KF0
   print "\n   Finding best fall-thru path:"
   for i in range(npath):
      KF1 = ' -p %d' %(i+1) + KF0
      fkocmnd.callfko(fko, KF1)
      [t,mf] = cmnd.time(ATLdir, ARCH, pre, blas, N, M, lda, "fkorout.s", 
                           "gcc", "-x assembler-with-cpp", opt=opt)
      print "      Path = %d, mflop = %.2f" % (i+1, mf)
      if mf > m0:
         m0 = mf
         t0 = t
         KFn = KF1

#
#  apply various reduction methods
#  [mmr, maxr, minr, rc] => [mmr, rc]
#
   if red1p[0] or red1p[1] or red1p[2] or red1p[3]:
      print "\n   Finding best path reduction:"
   if red1p[0] or red1p[1] or red1p[2] :
      if 'mmr' in skipOpt:
         print '      SKIPPING MMR'
      else:
         KF1 = ' -mmr' + KF0
         fkocmnd.callfko(fko, KF1)
         [t,mf] = cmnd.time(ATLdir, ARCH, pre, blas, N, M, lda, "fkorout.s", 
                              "gcc", "-x assembler-with-cpp", opt=opt)
         print "      MMR, mflop = %.2f" % ( mf)
         if mf > m0:
            m0 = mf
            t0 = t
            KFn = KF1 
   if red1p[3] :
#
#     skipping rc for cos kernel, there is a problem in cos!
#
      #if 'rc' in skipOpt or blas.find("cos") != -1 :
      if 'rc' in skipOpt != -1 :
         print '      SKIPPING RC'
      else: 
         KF1 = ' -rc' + KF0 
         fkocmnd.callfko(fko, KF1)
         [t,mf] = cmnd.time(ATLdir, ARCH, pre, blas, N, M, lda, "fkorout.s", 
                              "gcc", "-x assembler-with-cpp", opt=opt)
         print "      RC,  mflop = %.2f" % ( mf)
         if mf > m0:
            m0 = mf
            t0 = t
            KFn = KF1

   KF0 = KFn
   return [m0, KF0]


def ifko_Vec(ATLdir, ARCH, KF0, ncache, fko, rout, pre, blas, N, M, lda,  
             npath, vm, vpath):
   """ This function will try all vectorization methods incorporating other
   """
#
#  add vector flag if not in the flags
#
   if KF0.find('-V') == -1:
      KF0 = ' -V' + KF0
#
#  flag for speculation 
#
   global isSV
#
#  find out the standard vec flags
#
   #if SB:
   #   KF0 = GetOptStdFlags(fko, rout, pre, 1, SB);
   #else:
   #   KF0 = GetOptStdFlags(fko, rout, pre, 1);
   #print KF0
#
#  delete all flags which are associated with -p/rc/mmr
#
   j = KF0.find('-p')
   if j != -1:
      match = re.search(r'-p\s\d+\s',KF0) ## single instance
      if match:
         rem = match.group()
         KF0 = KF0.replace(rem, '') ## removing the -p val 
   j = KF0.find('-rc')
   if j != -1:
      KF0 = KF0.replace('-rc','')
   j = KF0.find('-mmr')
   if j != -1:
      KF0 = KF0.replace('-mmr','')

   print "\n   Finding best vectorization:"
#
#  check with other xforms
#
   m0 = 0
   t0 = 0
   KFn = KF0
   if npath > 1:
      if vm[0] or vm[1] or vm[2]:
         if 'mmr' in skipOpt or 'vrc' in forceOpt or 'sv' in forceOpt:
            print '      SKIPPING MMR+V'
         else:
            KF1 = ' -mmr' + KF0
            fkocmnd.callfko(fko, KF1)
            [t,mf] = cmnd.time(ATLdir, ARCH, pre, blas, N, M, lda, "fkorout.s", 
                                "gcc", "-x assembler-with-cpp", opt=opt)
            print "      MMR+V, mflop = %.2f" % ( mf)
            if mf > m0:
               m0 = mf
               t0 = t
               KFn = KF1
      if vm[3] :
         if 'rc' in skipOpt or 'vmmr' in forceOpt or 'sv' in forceOpt:
            print '      SKIPPING RC+V'
         else:
            KF1 = ' -rc' + KF0 
            fkocmnd.callfko(fko, KF1)
            [t,mf] = cmnd.time(ATLdir, ARCH, pre, blas, N, M, lda, "fkorout.s", 
                                "gcc", "-x assembler-with-cpp", opt=opt)
            print "      RC+V,  mflop = %.2f" % ( mf)
            if mf > m0:
               m0 = mf
               t0 = t
               KFn = KF1
#
#     check for speculative 
#
      if vm[4] :
         if 'sv' in skipOpt or 'vmmr' in forceOpt or 'vrc' in forceOpt:
            print '      SKIPPING SPECULATIVE VECTORIZATION'
         else:
            for i in range(npath):
               if vpath[i] :
                  KF1 = ' -p %d' %(i+1) + KF0
                  if SB:
                     KF1 = ' -B %d ' %(SB) + KF1
                  fkocmnd.callfko(fko, KF1)
                  [t,mf] = cmnd.time(ATLdir, ARCH, pre, blas, N, M, lda, "fkorout.s", 
                                      "gcc", "-x assembler-with-cpp", opt=opt)
                  if SB:
                     print "      V+SP%d SB=%d, mflop = %.2f" % (i+1, SB, mf)
                  else:
                     print "      V+SP%d, mflop = %.2f" % (i+1, mf)
                  if mf > m0:
                     m0 = mf
                     t0 = t
                     KFn = KF1
                     isSV = 1  ## SV is superior, so is applied from now
   else:    ## no path to reduce 
      fkocmnd.callfko(fko, KF0)
      [t,mf] = cmnd.time(ATLdir, ARCH, pre, blas, N, M, lda, "fkorout.s", 
                           "gcc", "-x assembler-with-cpp", opt=opt)
      print "      V, mflop = %.2f" % (mf)
      if mf > m0:
         m0 = mf
         t0 = t
         KFn = KF0

   KF0 = KFn
   return [m0,KFn]

def FindBET(ATLdir, ARCH, KF0, fko, rout, pre, blas, N, M, lda):
#
#  figure out the UR factor from flag
#
   j = KF0.find('-U')
   if j != -1:
      words = KF0[j:].split() 
      UR0 = int(words[1])
   else:
      UR0 = 1
#
#  set max bet as the UR but not less than 4, 
#  but not greater than 10
#
   #maxbet = UR0 * 2;
   maxbet = UR0
   if maxbet < 4:
      maxbet = 4
   if maxbet > 10:
      maxbet = 10
#
#  reduce tries for sine and cosine 
#
   if blas.find("sin") != -1:
      URm = 8;
   elif blas.find("cos") != -1:
      URm = 8;
   else:
      URm = 16 
#
#  time default case 
#
   fkocmnd.callfko(fko, KF0)
   [t0,m0] = cmnd.time(ATLdir, ARCH, pre, blas, N, M, lda, "fkorout.s", 
                         "gcc", "-x assembler-with-cpp", opt=opt)
   KFn = KF0
   KF1 = KF0
   mf1 = m0
   UR = 1
   SB = 1

   print "   Finding Over Speculation Factor, UR=%d, mflop= %.2f" % (UR0, m0)
   
   for i in range (2, maxbet):
#
#  remove previous bet if exists
#
      match = re.search(r'-B\s\d+',KFn) ## should not put space \s at last!
      if match:
         pat = match.group()
         KFn = KFn.replace(pat, '') ## remove all bet

      KFn = '-B %d ' %i + KFn
#
#     SB * UR should not be greater than 32 
#
      URN = int (URm/i) 
      for j in range (1, URN+1):
#
#        remove prev unroll factor
#
         match = re.search(r'-U\s\d+',KFn) ## should not put space \s at last!
         if match:
            pat = match.group()
            KFn = KFn.replace(pat, '') ## remove all bet
      
         KFn = '-U %d ' %j + KFn
      
         fkocmnd.callfko(fko, KFn)
         [t,mf] = cmnd.time(ATLdir, ARCH, pre, blas, N, M, lda, "fkorout.s", 
                           "gcc", "-x assembler-with-cpp", opt=opt)
         
         print "      SB=%d, UR=%d, mflop= %.2f" % (i, j, mf)
         
         if mf > mf1*1.001:
            mf1 = mf
            KF1 = KFn
            SB = i
            UR = j

   if mf1 > m0:
      KFn = KF1
      print "   Over Speculation : SB=%d, UR=%d, mf=%2.f, KFn=%s" % (SB, UR, 
             mf1, KFn)
   else:
      print "   No Over Speculation Selected!\n" 
      KFn = KF0
      mf1 = m0

   return [mf1, KFn]

def ifko0(l1bla, pre, N, M=None, lda=None):
   (IFKOdir, fko) = fkocmnd.GetFKOinfo()
   (ATLdir, ARCH) = fkocmnd.FindAtlas(IFKOdir)
   rout =  IFKOdir + '/blas/' + pre + l1bla + '.b'
   #outf =  ATLdir + '/tune/blas/level1/' + l1bla.upper() + '/fkorout.s'
   outf =  ATLdir + kernels.GetBlasPath(l1bla) + '/fkorout.s'
#
#  Majedul: calling new info func, info represents the old list
#  new data: [npath, red2onePath, vecMethod, vpathinfo, arrtypes] at the end
#
   #info = fkocmnd.info(fko, rout)

   newinfo = fkocmnd.NewInfo(fko, rout)
   info = [newinfo[i] for i in range(11) ]
   [npath, red1path, vecm, vpath, arrtypes] = [ newinfo[i] for i in range(11,16)]
   
   ncache = info[0]
   vec = info[5]
   (fparrs, fpsets, fpuses) = fkocmnd.GetFPInfo(info)
   nfp = len(fparrs)
#
#  Findout the default flags (it includes vector, default prefetch and unroll)
#
   #KFLAGS = fkocmnd.GetStandardFlags(fko, rout, pre) 
   KFLAGS = fkocmnd.GetOptStdFlags(fko, rout, pre, 1, URF) 
   #print "\n   Default Flag = " + KFLAGS 
   KFLAGS = KFLAGS + " -o " + str(outf) + " " + rout
   mflist = []
   testlist = []
   #print KFLAGS
#
#  Majedul: default and vect case would not be same now. Vspec may be 
#  worse than NonVec case.
#  So, I will choose the best as the default for the later optimization
#

#
#  check best scalar xforms, delete any vector flag
#
   #j = KFLAGS.find("-V")
   #if j != -1 :
      #KFn = KFLAGS[0:j-1] + KFLAGS[j+2:]
#
#  find out best standard scalar flag 
#
   KFn = fkocmnd.GetOptStdFlags(fko, rout, pre, 0, URF) 
   KFn = KFn + " -o " + str(outf) + " " + rout
   #print KFn
#
#  standard flag without vect
#
   KF0 = KFn
   fkocmnd.callfko(fko, KF0)
   [t0,mf0] = cmnd.time(ATLdir, ARCH, pre, l1bla, N, M, lda, "fkorout.s", 
                        "gcc", "-x assembler-with-cpp", opt=opt)
   mflist.append(mf0)
   testlist.append("default") ## this is using std flags
   print "\n   Default Flag = " + KF0 

#
#  Finding the best path reduction option
#
   if npath > 1:
      [mfs, KFs] = ifko_PathXform(ATLdir, ARCH, KFn, ncache, fko, rout, pre,
                                 l1bla, N, M, lda,  npath, red1path)
      mflist.append(mfs)
      testlist.append("PathXform")
      if (mfs > mf0) :
         mf0 = mfs
         KF0 = KFs
#
#  Finding the best vector option with/without path reduction
#
   global isSV;
   if SB:
      KFv = fkocmnd.GetOptStdFlags(fko, rout, pre, 1, SB, URF)
   else:
      KFv = fkocmnd.GetOptStdFlags(fko, rout, pre, 1, 0, URF)
   KFv = KFv + " -o " + str(outf) + " " + rout
   if vec:
      if 'v' in skipOpt:
         print '\n   SKIPPING VECTORIZATION'
      else:
         [mfv, KFv] =  ifko_Vec(ATLdir, ARCH, KFv, ncache, fko, rout, pre, 
                                l1bla, N, M, lda, npath, vecm, vpath)
         mflist.append(mfv)
         testlist.append("vect")
         if (mfv > mf0) :
            mf0 = mfv
            KF0 = KFv
#
#        if we have forceOpt, we will keep vec even if it's not better
#
         elif  'sv' in forceOpt or 'vrc' in forceOpt or 'vmmr' in forceOpt:
            print '\n   FORCING VECTORIZATION'
            mf0 = mfv
            KF0 = KFv
         else:  # no vector is selected, skip the SB too  #
            isSV = 0
#
#  choose the better as the ref of later opt
#
   KFLAGS = KF0
   mf = mf0
   
   print "\n   FLAGS so far =", fkocmnd.RemoveFilesFromFlags(l1bla, KFLAGS)

#
#  Previous code which is substituted by the above codes
#

#
#  Find performance of default case
#
#   j = KFLAGS.find("-V")
#   if j != -1 :
#      KFn = KFLAGS[0:j-1] + KFLAGS[j+2:]
#      fkocmnd.callfko(fko, KFn)
#      [t,mf] = l1cmnd.time(ATLdir, ARCH, pre, l1bla, N, "fkorout.s", 
#                           "gcc", "-x assembler-with-cpp", opt=opt)
#      mflist.append(mf)
#      testlist.append("default")
#      fkocmnd.callfko(fko, KFLAGS)
#      [t,mf] = l1cmnd.time(ATLdir, ARCH, pre, l1bla, N, "fkorout.s", 
#                           "gcc", "-x assembler-with-cpp", opt=opt)
#   else :
#      fkocmnd.callfko(fko, KFLAGS)
#      [t,mf] = l1cmnd.time(ATLdir, ARCH, pre, l1bla, N, "fkorout.s", 
#                           "gcc", "-x assembler-with-cpp", opt=opt)
#      testlist.append("default")
#      mflist.append(mf)
#   mflist.append(mf)
#   testlist.append("vect")
#
#  Eventually, want to try both -V and scalar, but for now, use -V whenever
#  possible

#
#  Find if we want to use cache-through writes on any arrays
#
   if 'wnt' in skipOpt:
      print '\n   SKIPPING WNT'
   else:
      n = len(fpsets)
      i = 0
      wnt = []
      while i < n:
         if fpsets[i] > 0 :
#        and fpuses[i] == 0:
            wnt.append(fparrs[i])
         i += 1
      if len(wnt) > 0:
         [mf,KFLAGS,wnt] = ifko_writeNT(ATLdir, ARCH, KFLAGS, fko, rout, pre,
                                     l1bla, N, M, lda, wnt)
      mflist.append(mf)
      testlist.append("writeNT") 
#
#  Find best PFD for each pfarr
#
   pfarrs = fparrs
   pfsets = fpsets
   for arr in pfarrs:
      [mf,KFLAGS] = FindPFD(ATLdir, ARCH, KFLAGS, fko, rout, pre,l1bla, N,M,lda,
                            info, arr)
   mflist.append(mf)
   testlist.append("pfdist")
   KFLAGS = fkocmnd.RemoveRedundantPrefFlags(KFLAGS, pfarrs)
#
#  Find best pf type
#
   [mf,KFLAGS] = ifko_pftype(ATLdir, ARCH, KFLAGS, ncache, fko, rout, pre, 
                             l1bla, N, M, lda, info, pfarrs, pfsets)
   mflist.append(mf)
   testlist.append("pftype")
   print "\n   FLAGS so far =", fkocmnd.RemoveFilesFromFlags(l1bla, KFLAGS)
   
#
#  Find best unroll
#
   if URF:
      print '\n   SKIPPING UNROLL TUNNING : FORCED TO %d' %URF
   else:
      [mf,KFLAGS] = FindUR(ATLdir, ARCH, KFLAGS, fko, rout, pre, l1bla, N, M, 
                            lda, info)
      mflist.append(mf)
      testlist.append("unroll")

#
#  Find best bet for over speculation
#  FIXME: find out the -U and pass it to the function
#  FIXME: can't apply Over Spec if there is a memory write inside the loop
#
   
   if isSV:
      if l1bla.find("irk1amax") != -1:
         print '\n   SKIPPING STRONGER BET UNROLLING for IRK1AMAX' 
      elif l1bla.find("irk2amax") != -1:
         print '\n   SKIPPING STRONGER BET UNROLLING for IRK2AMAX' 
      elif l1bla.find("irk3amax") != -1:
         print '\n   SKIPPING STRONGER BET UNROLLING for IRK3AMAX' 
      elif l1bla.find("sin") != -1:
         print '\n   SKIPPING STRONGER BET UNROLLING for SIN' 
      elif l1bla.find("cos") != -1:
         print '\n   SKIPPING STRONGER BET UNROLLING for COS' 
      else:
         [mf,KFLAGS] = FindBET(ATLdir, ARCH, KFLAGS, fko, rout, pre, l1bla, N, 
                               M, lda)
         mflist.append(mf)
         testlist.append("OverSpec")

#
#  See if we can apply accumulator expansion
#
#   acc = fkocmnd.GetFPAccum(info)
#   nacc = len(acc)
#   if nacc > 0 and nacc < 3:
#      [mf,KFLAGS] = FindAE(ATLdir, ARCH, KFLAGS, fko, rout, pre, l1bla, N, acc)
#   mflist.append(mf)
#   testlist.append("accexpans")

#
#  Majedul: See if we can apply scalar expansion (accexpan + man/min expansion)
#
   acc = fkocmnd.GetFPAccum(info)
   nacc = len(acc)
   if 'se' in skipOpt:
      print '\n   SKIPPING SCALAR EXPANSION'
   elif isSV:
      print '\n   SKIPPING SCALAR EXPANSION: NOT SUPPORTED WITH SV'
   else:
      if nacc > 0 and nacc < 3:
         [mf,KFLAGS] = FindSE(ATLdir, ARCH, KFLAGS, fko, rout, pre, l1bla, N, 
                              M, lda, acc)
      mflist.append(mf)
      testlist.append("sclexp")
#
#  Majedul: shifted it here to test
#
#
#  Find if we want to use cache-through writes on any arrays
#
   """if 'wnt' in skipOpt:
      print '\n   SKIPPING WNT'
   else:
      n = len(fpsets)
      i = 0
      wnt = []
      while i < n:
         if fpsets[i] > 0 :
#        and fpuses[i] == 0:
            wnt.append(fparrs[i])
         i += 1
      if len(wnt) > 0:
         [mf,KFLAGS,wnt] = ifko_writeNT(ATLdir, ARCH, KFLAGS, fko, rout, pre,
                                     l1bla, N, wnt)
      mflist.append(mf)
      testlist.append("writeNT") 
#
#  Find best PFD for each pfarr
#
   pfarrs = fparrs
   pfsets = fpsets
   for arr in pfarrs:
      [mf,KFLAGS] = FindPFD(ATLdir, ARCH, KFLAGS, fko, rout, pre,l1bla, N, 
                            info, arr)
   mflist.append(mf)
   testlist.append("pfdist")
   KFLAGS = fkocmnd.RemoveRedundantPrefFlags(KFLAGS, pfarrs)
#
#  Find best pf type
#
   [mf,KFLAGS] = ifko_pftype(ATLdir, ARCH, KFLAGS, ncache, fko, rout, pre, 
                             l1bla, N, info, pfarrs, pfsets)
   mflist.append(mf)
   testlist.append("pftype")
   print "\n   FLAGS so far =", fkocmnd.RemoveFilesFromFlags(l1bla, KFLAGS)
  """
#
#  tesing: re-tune the prefetch distance!
#  NOTE:  this re-tuning can be omitted just by enabling the comment
#
   #"""
   KFLAGS = fkocmnd.SetDefaultPFD(KFLAGS, info)
   #print "default PFD: ", KFLAGS
   print "\n   TUNING PFD AGAIN: "
   for arr in pfarrs:
      [mf,KFLAGS] = FindPFD(ATLdir, ARCH, KFLAGS, fko, rout, pre,l1bla, N, M, 
                            lda, info, arr)
   KFLAGS = fkocmnd.RemoveRedundantPrefFlags(KFLAGS, pfarrs)
#
# FIXME: it will create problem for the calculaton of % of improvement
#
#   if 'pfdist' in testlist:
#      j = testlist.index('pfdist')
#      mflist[j] = mf
#   else:
#      mflist.append(mf)
#      testlist.append("pfdist")
   #KFLAGS = fkocmnd.RemoveRedundantPrefFlags(KFLAGS, pfarrs)

   mflist.append(mf)
   testlist.append("pfd2")
   
   #"""

#
#  Find performance of best case
#
#   fkocmnd.callfko(fko, KFLAGS)
#   [t,mf] = l1cmnd.time(ATLdir, ARCH, pre, l1bla, N, "fkorout.s", 
#                        "gcc", "-x assembler-with-cpp", opt=opt)
   print "\n\n   BEST FLAGS FOUND (%.2f) = %s" % (mf,
         fkocmnd.RemoveFilesFromFlags(l1bla, KFLAGS))
   res = fkocmnd.GetOptVals(KFLAGS, pfarrs, pfsets, acc)
   tst = cmnd.test(ATLdir, ARCH, pre, l1bla, N, M, lda, "fkorout.s",
                     cc="gcc", ccf="-x assembler-with-cpp", opt=optT)
   #tst = l1cmnd.silent_test(ATLdir, ARCH, pre, l1bla, N, "fkorout.s",
   #                     cc="gcc", ccf="-x assembler-with-cpp", opt=optT)

   return(res, KFLAGS, mf, tst, testlist, mflist)


def ifko(routs, pres, N, M=None, lda=None):
   """ This function figures out the best parameter after tunning and print this
       out.
   """
   if routs.find("default") != -1:
      #routlist = l1cmnd.GetDefaultBlas()
      routlist = kernels.GetDefaultBlas()
   elif routs.find("all") != -1:
      routlist = kernels.GetAllKernels()
   elif routs.find("svkernels") != -1:
      #routlist = l1cmnd.GetSVKernels()
      routlist = kernels.GetSVKernels()
   elif routs.find("l1blas") != -1:
      routlist = kernels.GetLevel1Blas()
   elif routs.find("l2blas") != -1:
      routlist = kernels.GetLevel2Blas()
   elif routs.find("l3blas") != -1:
      routlist = kernels.GetLevel3Blas()
   else:
      words = routs.split(",")
      routlist = []
      for word in words:
          routlist.append(word)
   if pres.find("default") != -1:
      #prelist = l1cmnd.GetDefaultPre()
      prelist = kernels.GetDefaultPre()
   else:
      words = pres.split(",")
      prelist = []
      for word in words:
         prelist.append(word)

   reslist = []
   blalist = []
   mflist = []
   tstlist = []
   ideclist = []
   idecmflist = []
   for l1bla in routlist:
      for pre in prelist:
         print "\niFKO TUNING %s" % (pre + l1bla)
         (res,flags,mf,tst,dec,decmf) = ifko0(l1bla, pre, N, M, lda)
         reslist.append(res)
         blalist.append(pre + l1bla)
         mflist.append(mf)
         tstlist.append(tst)
         ideclist.append(dec)
         idecmflist.append(decmf)
   i = 0
   n = len(reslist)
#
#  Majedul:
#  Summerize the results of different opts
#  NOTE: this is to generate the string of latex syntax. As some optimization 
#  can be optional, need to redesign it when necessary
# 
#   while i < n :
#      mf0 = idecmflist[i][0]
#      (vec, UR, npf, pfinst, pfd, AE, WT) = reslist[i]
#      if vec: sv = "Y"
#      else : sv = "N"
#      mfsv = idecmflist[i][1]
#      mfsv = max(mf0, mfsv)
#
#      if len(WT) : wt = 'Y'
#      else : wt = 'N'
#      mfwt = max(mfsv,idecmflist[i][2])
#
#      pfdX = pfd[0]
#      if pfinst[0].find("none") != -1: pfIX = pfinst[0]
#      else : pfIX = r"{\tt " + pfinst[0][2:] + "}"
#      if npf == 2 :
#         pfdY = pfd[1]
#         if pfinst[1].find("none") != -1: pfIY = pfinst[1]
#         else : pfIY = r"{\tt " + pfinst[1][2:] + "}"
#      else :
#         pfdY = 0
#         pfIY = "N/A"
#      if len(AE) : ae = AE[0]
#      else : ae = 0
#      mfI  = max(mfwt,idecmflist[i][3])
#      mfpf = max(mfI,idecmflist[i][4])
#      mfur = max(mfpf,idecmflist[i][5])
#      mfae = max(mfur,idecmflist[i][6])

#      print "%10s & %1.1s (%.2f) & %1.1s (%.2f) & %s &%6d & %s (%.2f) &%6d (%.2f) & %d (%.2f) & %d (%.2f)\\\\\\hline" % \
#            (r"{\tt " + blalist[i]+"}", sv, mfsv/mf0, wt, mfwt/mfsv, pfIX,pfdX, 
#            pfIY, mfI/mfsv, pfdY, mfpf/mfsv, UR, mfur/mfsv, ae, mfae/mfsv)
#      i += 1


#
#  print the final result for each blas
#
   i = 0
   print "\n"
   while i < n :
      if tstlist[i] : tst = "FAIL"
      else : tst = "PASS"
      print "%10s : %5.5s %9.2f" % (blalist[i], tst, mflist[i])
      i += 1
#
#  print the table of result 
#
   i = 0
   print "\n"
#
#  FIXED: type of transformation can be varied from kernel to kernel now.
#  So, if parameter is default, need to show up all the transformation.
#
   if n > 1: ## when morethan one is applied, consider all xform
      StandardXform = ['default','PathXform','vect','writeNT','pfdist','pftype',
                      'unroll', 'sclexp', 'pfd2']
      m = len(StandardXform)
      st = "%7.7s" % ("BLAS")
      j = 1
      while j < m:
         st = st + " %6.6s" % (StandardXform[j])
         j += 1
      print st
      while i < n :
         st = "%7.7s" % blalist[i]
         j = 1 ## 1st one is always the default..
         #dmf = idecmflist[i][ideclist[i].index['default']]
         dmf = idecmflist[i][0]
         while j < m :
            if StandardXform[j] in ideclist[i]:
               mf = idecmflist[i][ideclist[i].index(StandardXform[j])] 
               st = st + " %6.2f" %(mf/dmf)
               dmf = mf
            else:
               st = st + "   --- " 
            j += 1
         print st
         i += 1
   else: ## previous one, consider only the xform which are applied
      m = len(ideclist[0])
      st = "%7.7s" % ("BLAS")
      j = 1
      while j < m:
         st = st + " %6.6s" % (ideclist[0][j])
         j += 1
      print st
      while i < n :
         st = "%7.7s" % blalist[i]
         j = 1
         while j < m :
            st = st + " %6.2f" % (idecmflist[i][j] / idecmflist[i][j-1])
            j += 1
         print st
         i += 1

def PrintUsage():
   print 'usage: '
   print 'Style1: ./ifko.py [asum, s]'
   print 'Style2: ./ifko.py default [s]'
   print 'Style3: ./ifko.py default default'
   print 'Style4: ./ifko.py (b1,b2,..) (s,d)'
   print 'Style5: ./ifko.py (b1,b2,..) (s,d) N'
   print 'Style6: ./ifko.py (b1,b2,..) (s,d) -n val'
   print 'Style7: ./ifko.py (b1,b2,..) (s,d) N --no <opt1,opt2,..>'
   print '        --force <sv, vrc, vmmx> atlopt=\'opt-str-for-atlas\' '
   print 'Style8: ./ifko.py (b1,b2,..) (s,d) -N val --no <opt1,opt2,..> ',
   print '-atlopt \'opt-str-for-atlas\' '
   print 'note that -atlopt \'opt-str-for-atlas\' should be last argument '
   return

def ParseArgv(argv):
   """ this function parses the commanline argument
   """
#
#  var init
#
   blas = None
   pre = None
   N = 0
   atlopt = None
   noOpt = []
   frOpt = []

   nargs = len(argv)
#
#  Set default parameters, if not set by user
#  Majedul: I kept backward compatibility 
#
   blas = 'asum'
   pre = 's'
   #N = 80000
   N = 16000
   M = 16000
   lda = 16000
#
#  tmp for SB testing, force UR
#
   global SB
   global URF
#
#  keep backward compatibility sothat it works with old arguments
#
   if nargs > 1:
      blas = argv[1]
      if nargs > 2:
         pre = argv[2]
         if nargs > 3:
            i = 3
            if argv[3].isdigit() :
               N = int(sys.argv[3])
               i = i + 1
            while i < nargs: 
               if argv[i].find('--no') != -1:
                  noOpt = argv[i+1].split(',')
                  i = i + 2
               elif argv[i].find('--force') != -1:
                  frOpt = argv[i+1].split(',')
                  i = i + 2
               elif argv[i].find('-n') != -1:
                  N = int(argv[i+1])
                  i = i + 2
               elif argv[i].find('-m') != -1:
                  M = int(argv[i+1])
                  i = i + 2
               elif argv[i].find('-lda') != -1:
                  lda = int(argv[i+1])
                  i = i + 2
               elif argv[i].find('-s') != -1:
                  SB = int(argv[i+1])
                  i = i + 2
               elif argv[i].find('-u') != -1:
                  URF = int(argv[i+1])
                  i = i + 2
               elif argv[i].find('-atlopt') != -1:
                  print argv
                  #print argv[j]
                  atlopt = ' '.join([argv[j] for j in range(i+1, nargs)])
            #argument with ' ' behave differently with system to system
                  print atlopt
                  if atlopt.find('\'') != -1:
                     atlopt = atlopt.split('\'')[1]
                     i = i + 2
                  else:
                     #print "Error in atlopt format!"
                     #system.exit(1)
                     #else:
                        #atlopt = atlopt[atlopt.find('=')+1:]
                     break; ## stop checking any other argment
               else:
                  PrintUsage()
                  sys.exit(1)
#   if atlopt is None:
#      print [blas, pre, N, noOpt]
#   else:
#      print [blas, pre, N, noOpt, atlopt]
   
   return(blas,pre,N,M,lda,noOpt,frOpt,atlopt) 
#
#  Majedul: create a main function to increase the readability
#
def main(argv):
   """This function will only be executed when the program is being run by 
      itself and not imported by others. it will examin the commnad line 
      argument and run the ifko functtion with blas, pre, N, uopt
   """
   nargs = len(argv)
#
#  Majedul: new design for command line argument, make it fexible as possible
#  with default values but keep backward compatibility ... 
#  Argument format :
#     blas pre N 
#     blas, pre, N=value, opt=value, --nopt=opt1,opt2, ...
#  (blas, pre) is must, other args are optional 
#

#
#  parse the commnad line argument 
#
   (blas, pre, N, M, lda, noOpt, frOpt, uopt) = ParseArgv(argv)
#
#  Generate opt for altas  based on the argument 
#
   global skipOpt
   global forceOpt
   
   skipOpt = noOpt
   forceOpt = frOpt
   #print skipOpt

   global opt      
   if blas == 'l3blas' or blas in kernels.GetLevel3Blas():
      opt = ""
   elif uopt is None:
      #opt = "-C 1 -X 1 -Y 1 -Fx 32 -Fy 32 -Fa 32"  ## default opt
      opt = "-C 1 -Fx 32 -Fy 32 -Fa 32"  ## default opt
   else:
      #if uopt.find('-X') is -1:  ## returns -1 if not found, otherwise index
      #   opt +='-X 1 '
      #if uopt.find('-Y') is -1:
      #   opt +='-Y 1 '
      if uopt.find('-C') is -1:
         opt +='-C 1 '           ## no cache flushing by default 
      if uopt.find('-Fx') is -1:
         opt +='-Fx 32 '         ## default for AVX 
      if uopt.find('-Fy') is -1:
         opt +='-Fy 32 '
      if uopt.find('-Fa') is -1:
         opt +='-Fa 32 '
      opt += uopt
   
   #print [blas, pre, N, noOpt, opt]
   #sys.exit(0)
   print N, M, lda, opt
   ifko(blas, pre, N, M, lda)

#
# Majedul: If the program is being run by itself and not imported by others, 
# run the main function only then
#

if __name__ == '__main__':
   main(sys.argv)
