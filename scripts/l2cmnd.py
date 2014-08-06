import os
import sys
import re
import gc

WALLTIME = 1
GEMVTdir = '/tune/blas/gemv/MNVTCASES/'
GEMVNdir = '/tune/blas/gemv/MNVNCASES/'
GER1dir =  '/tune/blas/ger/R1CASES/'
GER2dir =  '/tune/blas/ger/R2CASES/'

def GetDefaultBlas():
   return ['gemv', 'gemvt', '']

def GetDefaultRefBlas(blas):
   n = len(blas)
   assert(n)
   refb = []
   for bla in blas:
      if bla.find('gemvt') != -1:
         refb.append("gemvt_c.c")
      else:
         print "Unknown BLAS : %s" % (bla)
         sys.exit(1)

   return refb

def GetDefaultCblasBlas(blas):
   n = len(blas)
   assert(n)
   refb = []
   for bla in blas:
      if bla.find('gemvt') != -1:
         refb.append("gemvt_c.c")
      else:
         print "Unknown BLAS : %s" % (bla)
         sys.exit(1)

   return refb

def GetDefaultPre():
   return ['s', 'd']

def test(ATLdir, ARCH, pre, blas, N, M, lda, rout, cc=None, ccf=None, opt=""):

    ERR = 0
    if (opt != ""):
      opt = 'align="' + opt + '"'
#
#   set parameters based on kernela
#
    if blas.find('gemvt') != -1:
        tdir = '/tune/blas/gemv/'
        target = pre + 'mvtktest'
        rout = 'mvtrout=' + rout
        Nv = 'Nt=' + str(N)
        Mv = 'Mt=' + str(M)
        LDAv = 'ldat=' + str(lda)
        if (cc != None):
            opt = pre + 'MVFLAGS="' + ccf +'" ' + opt
            opt = pre + 'MVCC=' + cc +' ' + opt
    elif blas.find('gemvn') != -1:
        tdir = '/tune/blas/gemv/'
        target = pre + 'mvnktest'
        rout = 'mvnrout=' + rout
        Nv = 'Nt=' + str(N)
        Mv = 'Mt=' + str(M)
        LDAv = 'ldat=' + str(lda)
        if (cc != None):
            opt = pre + 'MVFLAGS="' + ccf +'" ' + opt
            opt = pre + 'MVCC=' + cc +' ' + opt
    elif blas.find('ger1') != -1:
        tdir = '/tune/blas/ger/'
        target = pre + 'r1ktest'
        rout = 'r1rout=' + rout
        Nv = 'Nt=' + str(N)
        Mv = 'Mt=' + str(M)
        LDAv = 'ldat=' + str(lda)
        if (cc != None):
            opt = pre + 'R1CFLAGS="' + ccf +'" ' + opt
            opt = pre + 'R1CC=' + cc +' ' + opt
    elif blas.find('ger2') != -1:
        tdir = '/tune/blas/ger/'
        target = pre + 'r2ktest'
        rout = 'r2rout=' + rout
        Nv = 'Nt=' + str(N)
        Mv = 'Mt=' + str(M)
        LDAv = 'ldat=' + str(lda)
        if (cc != None):
            opt = pre + 'R2CFLAGS="' + ccf +'" ' + opt
            opt = pre + 'R2CC=' + cc +' ' + opt
    else:
        print 'KERNEL NOT SUPPORTED YET\n'
        sys.exit(1)
    
#
#  Majedul: for single, we need to use sUCCFLAGS   
#
#    if (cc != None):
#        opt = opt + ' ' + pre + 'MVCC=' + cc + ' '+ pre + 'MVFLAGS="' + ccf + '"'
    cmnd = 'cd %s%s ; make %s %s %s %s %s %s' % \
          (ATLdir, tdir, target, Nv, Mv, LDAv, rout, opt)
  
    #print cmnd 
   
    fo = os.popen(cmnd, 'r')
    lines = fo.readlines()
    err = fo.close()
    if (err != None):
        print 'command died with:'
        print lines
        print err
        return err
    for line in lines:
        if line.find("FAILED") != -1:
            #ERR = 1
            #break
            return 1
    
    #print lines 
    #return ERR
    return 0

#
#  Majedul: silent test with no err msg 
#
def silent_test(ATLdir, ARCH, pre, blas, N, M, lda, rout, cc=None, ccf=None, opt=""):
   return 0 

def time(ATLdir, ARCH, pre, blas, N, M, lda, rout, cc=None, ccf=None, opt=""):
#
#  Extract the original seconds & summation time & mflop
#

    if (opt != ""):
      opt = 'align="' + opt + '"'
    else:
      opt = 'align="-Fx 32 -Fy 32"'
    
    if blas.find('gemvt') != -1:
        tdir = '/tune/blas/gemv/'
        target = pre + 'mvtktime'
        rout = 'mvtrout=' + rout
        Nv = 'Nt=' + str(N)
        Mv = 'Mt=' + str(M)
        LDAv = 'ldat=' + str(lda)
        if (cc != None):
            opt = pre + 'MVFLAGS="' + ccf +'" ' + opt
            opt = pre + 'MVCC=' + cc +' ' + opt
    elif blas.find('gemvn') != -1:
        tdir = '/tune/blas/gemv/'
        target = pre + 'mvnktime'
        rout = 'mvnrout=' + rout
        Nv = 'N=' + str(N)
        Mv = 'M=' + str(M)
        LDAv = 'lda=' + str(lda)
        if (cc != None):
            opt = pre + 'MVFLAGS="' + ccf +'" ' + opt
            opt = pre + 'MVCC=' + cc +' ' + opt
    elif blas.find('ger1') != -1:
        tdir = '/tune/blas/ger/'
        target = pre + 'r1ktime'
        rout = 'r1rout=' + rout
        Nv = 'N=' + str(N)
        Mv = 'M=' + str(M)
        LDAv = 'lda=' + str(lda)
        if (cc != None):
            opt = pre + 'R1CFLAGS="' + ccf +'" ' + opt
            opt = pre + 'R1CC=' + cc +' ' + opt
    elif blas.find('ger2') != -1:
        tdir = '/tune/blas/ger/'
        target = pre + 'r2ktime'
        rout = 'r2rout=' + rout
        Nv = 'N=' + str(N)
        Mv = 'M=' + str(M)
        LDAv = 'lda=' + str(lda)
        if (cc != None):
            opt = pre + 'R2CFLAGS="' + ccf +'" ' + opt
            opt = pre + 'R2CC=' + cc +' ' + opt
    else:
        print 'KERNEL NOT SUPPORTED YET\n'
        sys.exit(1)
    
    #print opt
      
#    PROFILE = 1
#   if cc != None and cc.find("iccprof") != -1: PROFILE = 1
#   else : PROFILE = 0
#   if PROFILE:
#      popt = opt + ' UCC=icc UCCFLAGS="' + ccf + ' -prof_genx -prof_dir /tmp"'
#      pcmnd = 'rm -f /tmp/*.dyn ; cd %s/tune/blas/level1/%s ; make %s%scase N=%d urout=%s %s' % \
#      pcmnd = 'rm -f /tmp/*.dyn ; cd %s/tune/blas/level1 ; make %s%scase N=%d urout=%s %s' % \
#             (ATLdir, pre, blas, N, rout, popt)
#      print "cmnd = '%s'" % (pcmnd)
#      fo = os.popen(pcmnd, 'r')
#      lines = fo.readlines()
#      err = fo.close()
#      print lines
#      assert(err == None) 

#
#     Majedul: there is no variable named UCCFLAGS. it should be sUCCFLAGS or
#     dUCCFLAGS. Same as UCC. 
#
#      zopt = opt + ' UCC=icc UCCFLAGS="' + ccf + ' -prof_use -prof_dir /tmp"'
#   if PROFILE:
#      opt = zopt
#   elif cc != None :
      #opt = opt + ' UCC=' + cc + ' UCCFLAGS="' + ccf + '"'       
#      opt = opt + ' '+ pre + 'UCC=' + cc + ' '+ pre + 'UCCFLAGS="' + ccf + '"'     
#   print "opt = '%s'" % opt
#   cmnd = 'make %s%scase N=%d urout=%s %s' % (pre, blas, N, rout, opt)
#   if WALLTIME and 0:
#      cmnds = 'cd %s/tune/blas/level1/%s ; %s ; %s ; %s' % (ATLdir, ARCH, 
#      cmnds = 'cd %s/tune/blas/level1 ; %s ; %s ; %s' % (ATLdir,  
#              cmnd, cmnd, cmnd)
#   elif PROFILE and cc == None:
#      cmnds = 'cd %s/tune/blas/level1/%s ; %s ' % (ATLdir, ARCH, cmnd)
#   else :
#      cmnds = 'cd %s/tune/blas/level1/%s ; %s ; %s' % (ATLdir, ARCH, cmnd, cmnd)
#      cmnds = 'cd %s/tune/blas/level1; %s ; %s' % (ATLdir, cmnd, cmnd)
    
    cmnds = 'cd %s%s ; make %s %s %s %s %s %s' % \
        (ATLdir, tdir, target, Nv, Mv, LDAv, rout, opt)
    gc.disable()
    fo = os.popen(cmnds, 'r')
    lines = fo.readlines()
    err = fo.close()
    gc.enable()
    if (err != None):
        print 'command died with: %d' % (err)
        print cmnds
        return [-1.0, -1.0]
#      sys.exit(err)

    l2sum = re.compile(r"time=(.*), mflop=(.*)\s.*$")
    t=[]
    m=[]
    for line in lines:
        match = re.search(l2sum, line)
        if (match):
            t.append (float(match.group(1)))
            m.append (float(match.group(2)))

# NOTE: normally we pick minimum time for WALLTIME, but here we compute the 
# avg 
    tim = t[0]
    mf = m[0]
    t.sort()
    WALLTIME = 0
    if WALLTIME: 
        mf = (mf*tim)/t[0]
        tim = t[0]
    else:
#       calc avg value         
        mf = (m[0]+m[1]+m[2])/3
        tim = (t[0]+t[1]+t[2])/3
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
