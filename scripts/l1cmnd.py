import os
import sys
import re

L1Tdir = '/tune/blas/level1/'

def time(ATLdir, ARCH, pre, blas, N, rout, cc=None, ccf=None, opt=""):
#
#  Extract the original seconds & summation time & mflop
#
   l1sec = re.compile(r"tim=(.*)\s.*$")
   l1sum = re.compile(r"N=(.*),\s.*time=(.*), mflop=(.*)\s.*$")

   if (opt != ""):
      opt = 'opt="' + opt + '"'
   if(cc != None):
      opt = opt + ' UCC=' + cc + ' UCCFLAGS="' + ccf + '"'
#   print "opt = '%s'" % opt
   cmnd = 'make %s%scase N=%d urout=%s %s' % (pre, blas, N, rout, opt)
#   print "cmnd= '%s'" % cmnd
   cmnds = 'cd %s/tune/blas/level1/%s ; %s ; %s' % (ATLdir, ARCH, cmnd, cmnd)
   fo = os.popen(cmnds, 'r')
   lines = fo.readlines()
   err = fo.close()
   if (err != None):
      print 'command died with:'
      print err
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
   mf = (mfavg * tavg) / t[2]
   return [t[2], mf]

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
   file = ATLdir + L1Tdir + ARCH + '/res/' + pre + rout.upper() + '_SUMM'
   fi = open(file, 'r')
   line = fi.readline()
   line = fi.readline()
   fi.close()
   words = line.split()
   id = int(words[0])
   [CC, CCF] = getucc(ATLdir, ARCH, rout, id, pre)
   return [ int(words[0]), words[5], CC, CCF ]
