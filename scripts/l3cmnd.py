import os
import subprocess
import sys
import re
import gc
import kernels

WALLTIME = 1
GEMMdir =  '/tune/blas/gemm/CASES'

def GetDefaultBlas():
   return ['gemm']

def GetDefaultRefBlas(blas):
   n = len(blas)
   assert(n)
   refb = []
   for bla in blas:
      if bla.find('gemm') != -1:
         refb.append("gemm_c.c")
      else:
         print "Unknown BLAS : %s" % (bla)
         sys.exit(1)

   return refb

def GetDefaultCblasBlas(blas):
    n = len(blas)
    assert(n)
    refb = []
    for bla in blas:
        if bla.find('gemm') != -1:
            refb.append("gemm_c.c")
        else:
            print "Unknown BLAS : %s" % (bla)
            sys.exit(1)
    return refb


def GetDefaultPre():
   return ['s', 'd']

def test(ATLdir, ARCH, pre, blas, NB, beta, rout, cc=None, ccf=None, opt=""):

    #if (opt != ""):
    #  opt = 'align="' + opt + '"'
#
#       remove unnecessary flags for opt, e.g.: -X 1 1 -Y 1 1 -Fx 32 -Fy 32 
#
    matches = re.findall(r'-F[x|y|a]\s[-]?\d+',opt)
    for match in matches:
       opt = opt.replace(match, '')
        
    matches = re.findall(r'-[X|Y]\s[-]?\d+\s[-]?\d+',opt)
    for match in matches:
       opt = opt.replace(match, '')
   
    matches = re.findall(r'-C\s[-]?\d+',opt)
    for match in matches:
       opt = opt.replace(match, '')
#
#   set parameters based on kernel
#
    #if blas.find('gemm') != -1:
    if blas in kernels.GetAllFKOGEMM():
        tdir = '/tune/blas/gemm/'
        target = 'mmutstcase pre=' + pre
        rout = 'mmrout=CASES/' + rout
        NB = 'nb=' + str(NB)
        beta ='beta=' + str(beta)
        if (cc != None):
            opt = 'DMCFLAGS="' + ccf +'" ' + opt
            opt = 'DMC=' + cc +' ' + opt
    else:
        print 'KERNEL NOT SUPPORTED YET\n'
        sys.exit(1)
    
#
#  Majedul: for single, we need to use sUCCFLAGS   
#
#    if (cc != None):
#        opt = opt + ' ' + pre + 'MVCC=' + cc + ' '+ pre + 'MVFLAGS="' + ccf + '"'
    cmnd = 'cd %s%s ; make %s %s %s %s %s ' % \
          (ATLdir, tdir, target, NB, rout, opt, beta)
  
    #print cmnd 
    #try: 
    #  fo = os.popen(cmnd, 'r')
    #  lines = fo.readlines()
    #  err = fo.close()
    #except OSError:
    #  pass
    
    proc = subprocess.Popen(cmnd, shell=True, stderr=subprocess.PIPE, 
          stdout=subprocess.PIPE)
    return_codes = proc.wait()
    lines = proc.stdout
#
#   -ve value of return code indicate that the child proc was terminated with
#   signal N in Unix
#
    if (return_codes != 0):
        print 'command died with:'
        print cmnd
        print lines 
        print return_codes 
        return return_codes

    ERR = 1
    for line in lines:
        if line.find("PASSED TEST") != -1:
            ERR = 0
            #break
            #return 1
    
    #print lines 
    #return ERR
    return ERR

#
#  Majedul: silent test with no err msg 
#
def silent_test(ATLdir, ARCH, pre, blas, N, M, lda, rout, cc=None, ccf=None, opt=""):
   return 0 

def time(ATLdir, ARCH, pre, blas, NB, beta, rout, cc=None, ccf=None, opt=""):
#
#  Extract the original seconds & summation time & mflop
#

#    if (opt != ""):
#      opt = 'align="' + opt + '"'
#    else:
#      opt = 'align="-Fx 32 -Fy 32"'

#
#       remove unnecessary flags for opt, e.g.: -X 1 1 -Y 1 1 -Fx 32 -Fy 32 
#
   matches = re.findall(r'-F[x|y|a]\s[-]?\d+',opt)
   for match in matches:
      opt = opt.replace(match, '')
        
   matches = re.findall(r'-[X|Y]\s[-]?\d+\s[-]?\d+',opt)
   for match in matches:
      opt = opt.replace(match, '')
    
   matches = re.findall(r'-C\s[-]?\d+',opt)
   for match in matches:
      opt = opt.replace(match, '')
   
   #if blas.find('gemm') != -1:
   if blas in kernels.GetAllFKOGEMM():
      tdir = '/tune/blas/gemm/'
      target = 'ummcase pre=' + pre
      rout = 'mmrout=CASES/' + rout
      NB = 'nb=' + str(NB)
      beta = 'beta=' + str(beta)
      if (cc != None):
         opt = 'DMCFLAGS="' + ccf +'" ' + opt
         opt = 'DMC=' + cc +' ' + opt
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
    
    #cmnds = 'cd %s%s ; make %s %s %s %s %s %s' % \
    #    (ATLdir, tdir, target, Nv, Mv, LDAv, rout, opt)
   cmnds = 'cd %s%s ; make %s %s %s %s %s ' % \
          (ATLdir, tdir, target, NB, rout, opt, beta)
   gc.disable()
    
#    try:
#      fo = os.popen(cmnds, 'r')
#      lines = fo.readlines()
#      err = fo.close()
#      gc.enable()
#    except OSError:
#      pass

   proc = subprocess.Popen(cmnds, shell=True, stderr=subprocess.PIPE, 
          stdout=subprocess.PIPE)
   return_codes = proc.wait()
   lines = proc.stdout
   gc.enable()
#
#   -ve value of return code indicate that the child proc was terminated with
#   signal N in Unix
#
   #print 'return codes = ', return_codes
   if (return_codes != 0):
      print 'command died with: %d' % (return_codes)
      print cmnds
      print proc.stderr
      #print proc.stdout
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

    #print m, t
# NOTE: for gemm the final result is output at the end of the lines
# no need to avg them
#    tim = t[0]
#    mf = m[0]
#    t.sort()
#    WALLTIME = 0
#    if WALLTIME: 
#        mf = (mf*tim)/t[0]
#        tim = t[0]
#    else:
#       calc avg value         
#        mf = (m[0]+m[1]+m[2])/3
#        tim = (t[0]+t[1]+t[2])/3
#    return [tim, mf]
   return [t[0], m[0]]


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
