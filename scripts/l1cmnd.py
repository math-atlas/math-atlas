import os
import sys
import re
import gc

WALLTIME = 1
L1Tdir = '/tune/blas/level1/'

def GetDefaultBlas():
   return ['swap', 'copy', 'asum', 'axpy', 'dot', 'scal', 'iamax', 'amax', 
           'nrm2']

def GetDefaultRefBlas(blas):
   n = len(blas)
   assert(n)
   refb = []
   for bla in blas:
      if bla.find('asum') != -1:
         refb.append("asum_fabs1_x1.c")
      elif bla.find('axpy') != -1:
         refb.append("axpy1_x1y1.c")
      elif bla.find('dot') != -1:
         refb.append("dot1_x1y1.c")
      elif bla.find('scal') != -1:
         refb.append("scal1_x1.c")
      elif bla.find('amax') != -1:
#         refb.append("iamax_jmp_x1.c")
         refb.append("iamax_abs1_x1.c")
      elif bla.find("copy") != -1:
         refb.append("copy1_x1y1.c")
      elif bla.find("swap") != -1:
         refb.append("swap1_x1y1.c")
      else:
         print "Unknown BLAS : %s" % (bla)
         sys.exit(1)

   return refb

def GetDefaultCblasBlas(blas):
   n = len(blas)
   assert(n)
   refb = []
   for bla in blas:
      if bla.find('asum') != -1:
         refb.append("asum_blas.c")
      elif bla.find('axpy') != -1:
         refb.append("axpy_blas.c")
      elif bla.find('dot') != -1:
         refb.append("dot_blas.c")
      elif bla.find('scal') != -1:
         refb.append("scal_blas.c")
      elif bla.find('amax') != -1:
#         refb.append("iamax_blas.c")
         refb.append("iamax_blas.c")
      elif bla.find("copy") != -1:
         refb.append("copy_blas.c")
      elif bla.find("swap") != -1:
         refb.append("swap_blas.c")
      else:
         print "Unknown BLAS : %s" % (bla)
         sys.exit(1)

   return refb

def GetDefaultPre():
   return ['s', 'd']

def test(ATLdir, ARCH, pre, blas, N, rout, cc=None, ccf=None, opt=""):
   if (opt != ""):
      opt = 'opt="' + opt + '"'
#
#  Majedul: for single, we need to use sUCCFLAGS   
#
   if(cc != None):
      #opt = opt + ' dUCC=' + cc + ' dUCCFLAGS="' + ccf + '"'
      opt = opt + ' ' + pre + 'UCC=' + cc + ' '+ pre + 'UCCFLAGS="' + ccf + '"'
#   cmnd = 'cd %s/tune/blas/level1/%s ; make %s%stest N=%d urout=%s %s' % \
#          (ATLdir, ARCH, pre, blas, N, rout, opt)
   cmnd = 'cd %s/tune/blas/level1 ; make %s%stest N=%d urout=%s %s' % \
          (ATLdir, pre, blas, N, rout, opt)
  
   
   fo = os.popen(cmnd, 'r')
   lines = fo.readlines()
   err = fo.close()
   if (err != None):
      print 'command died with:'
      print lines
      print err
      return err
   for line in lines:
      if line.find("ALL") != -1 and line.find("SANITY TESTS PASSED") != -1:
         break
   else:
      return 1
   return 0 

#
#  Majedul: silent test with no err msg 
#
def silent_test(ATLdir, ARCH, pre, blas, N, rout, cc=None, ccf=None, opt=""):
   if (opt != ""):
      opt = 'opt="' + opt + '"'
   if(cc != None):
      opt = opt + ' ' + pre + 'UCC=' + cc + ' '+ pre + 'UCCFLAGS="' + ccf + '"'
   cmnd = 'cd %s/tune/blas/level1 ; make %s%stest N=%d urout=%s %s' % \
          (ATLdir, pre, blas, N, rout, opt)
   try:
      fo = os.popen(cmnd, 'r')
      lines = fo.readlines()
      err = fo.close()
   except OSError:
      # skip the err msg !
      pass

   if (err != None):
      return err
   for line in lines:
      if line.find("ALL") != -1 and line.find("SANITY TESTS PASSED") != -1:
         break
   else:
      return 1
   return 0 

def time(ATLdir, ARCH, pre, blas, N, rout, cc=None, ccf=None, opt=""):
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
      
   if PROFILE:
      popt = opt + ' UCC=icc UCCFLAGS="' + ccf + ' -prof_genx -prof_dir /tmp"'
#      pcmnd = 'rm -f /tmp/*.dyn ; cd %s/tune/blas/level1/%s ; make %s%scase N=%d urout=%s %s' % \
      pcmnd = 'rm -f /tmp/*.dyn ; cd %s/tune/blas/level1 ; make %s%scase N=%d urout=%s %s' % \
             (ATLdir, pre, blas, N, rout, popt)
      print "cmnd = '%s'" % (pcmnd)
      fo = os.popen(pcmnd, 'r')
      lines = fo.readlines()
      err = fo.close()
#      print lines
      assert(err == None) 
#
#     Majedul: there is no variable named UCCFLAGS. it should be sUCCFLAGS or
#     dUCCFLAGS. Same as UCC. 
#
      zopt = opt + ' UCC=icc UCCFLAGS="' + ccf + ' -prof_use -prof_dir /tmp"'
   if PROFILE:
      opt = zopt
   elif cc != None :
      #opt = opt + ' UCC=' + cc + ' UCCFLAGS="' + ccf + '"'       
      opt = opt + ' '+ pre + 'UCC=' + cc + ' '+ pre + 'UCCFLAGS="' + ccf + '"'     
#   print "opt = '%s'" % opt
   cmnd = 'make %s%scase N=%d urout=%s %s' % (pre, blas, N, rout, opt)
   if WALLTIME and 0:
#      cmnds = 'cd %s/tune/blas/level1/%s ; %s ; %s ; %s' % (ATLdir, ARCH, 
      cmnds = 'cd %s/tune/blas/level1 ; %s ; %s ; %s' % (ATLdir,  
              cmnd, cmnd, cmnd)
#   elif PROFILE and cc == None:
#      cmnds = 'cd %s/tune/blas/level1/%s ; %s ' % (ATLdir, ARCH, cmnd)
   else :
#      cmnds = 'cd %s/tune/blas/level1/%s ; %s ; %s' % (ATLdir, ARCH, cmnd, cmnd)
      cmnds = 'cd %s/tune/blas/level1; %s ; %s' % (ATLdir, cmnd, cmnd)
   gc.disable()
   fo = os.popen(cmnds, 'r')
   lines = fo.readlines()
   err = fo.close()
   gc.enable()
   if (err != None):
      print 'command died with: %d' % (err)
      print cmnd
      return [-1.0, -1.0]
#      sys.exit(err)

   t=[]

   for line in lines:
      match = re.search(l1sec, line)
      if (match):
         t.append (float(match.group(1)))
      else:
         match = re.search(l1sum, line)
         if (match):
            n = int(match.group(1))
            tavg = float(match.group(2))
            mfavg = float(match.group(3))
#
#  find mf from avg times
#
#   if (len(t) > 6):
#      print '**** WARNING: time = ', t
   t.sort()
   if WALLTIME: tim = t[0]
   else : tim = t[2]
   mf = (mfavg * tavg) / tim
   return [tim, mf]

def getucc(ATLdir, ARCH, rout, id, pre='d'):
   file = ATLdir + L1Tdir + rout.upper() + '/' + pre + 'cases.dsc'
   fi = open(file, 'r')
   line = fi.readline()
   CC = CCF = None
   while (1):
      line = fi.readline()
      if (not line):
         print "File='%s': no such ID (%d)!" % (file, id)
         sys.exit(-1)
      words = line.split()
      if (len(words) > 0 and words[0].isdigit()):
         i = int(words[0])
         if (i == id):
            j = len(words)
            st = words[j-1].strip()
            if (st.endswith('\\')):
               CC = fi.readline()
               CCF = fi.readline()
               CC = CC.strip()
               CCF = CCF.strip()
            break
   fi.close()
   return[CC, CCF]

def res(ATLdir, ARCH, rout, pre='d'):
#   file = ATLdir + L1Tdir + ARCH + '/res/' + pre + rout.upper() + '_SUMM'
   file = ATLdir + L1Tdir + '/res/' + pre + rout.upper() + '_SUMM'
   fi = open(file, 'r')
   line = fi.readline()
   line = fi.readline()
   fi.close()
   words = line.split()
   id = int(words[0])
   [CC, CCF] = getucc(ATLdir, ARCH, rout, id, pre)
   return [ int(words[0]), words[5], CC, CCF ]
