import os
import sys

def GetFKOinfo():
   pwd = os.getcwd()
   j = pwd.rfind('iFKO')
   IFKOdir = pwd[0:j+4]
   fko = IFKOdir + '/bin/fko'
   return(IFKOdir, fko)

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
   return(ATLdir, ARCH) 
#
# returns info from fko's analysis using -i
#
def info(fko, routine):
   cmnd = fko + ' -i stdout ' + routine
#   print cmnd
   fi = os.popen(cmnd, 'r')
   lines = fi.readlines()
   err = fi.close()
   if (err != None):
      print 'command died with:', err
      sys.exit(err)
   nc = int(lines[0][8:])
   words = lines[1].split()
   i = 0
   LS = []
   while (i < nc):
      LS.append(int(words[i+2]))
      i += 1

   scalinf = (0, [], [], [], [], [])
   arrs = None
   pref = None
   sets = None
   ol = int(lines[2][8:])
   if (ol != 0):
      maxunroll = int(lines[3][13:])
      lnf = int(lines[4][18:])
      vec = int(lines[5][16:])
      mfp = int(lines[6][23:])
      arrs = []
      pref = []
      sets = []
      uses = []
      assert(len(pref) == 0)
      i = 0;
      while (i < mfp):
         words = lines[7+i].split()
         j = len(words[0])
         arrs.append(words[0][1:j-2])
         j = words[2].rfind("=")
         pref.append(int(words[2][j+1:]))
         j = words[3].rfind("=")
         sets.append(int(words[3][j+1:]))
         j = words[4].rfind("=")
         uses.append(int(words[4][j+1:]))
         i += 1;
      ns = int(lines[7+mfp][25:])
      scal = []
      styp = []
      sset = []
      suse = []
      sacc = []
      i = 0
      while i < ns:
         words = lines[8+mfp+i].split()
         j = len(words[0])
         scal.append(words[0][1:j-2])
         j = words[1].rfind("=")
         styp.append(words[1][j+1])
         j = words[2].rfind("=")
         sset.append(int(words[2][j+1:]))
         j = words[3].rfind("=")
         suse.append(int(words[3][j+1:]))
         j = words[4].rfind("=")
         sacc.append(int(words[4][j+1:]))
         i += 1
      scalinf = (ns, scal, styp, sset, suse, sacc)
   else:
      maxunroll = lnf = vec = mfp = 0
   return[nc, LS, ol, maxunroll, lnf, vec, arrs, pref, sets, uses, scalinf]

def GetFPAccum(info) :
   scalinf = info[10]
   (ns, scal, styp, sset, suse, sacc) = scalinf
   acc = []
   i = 0
   while i < ns:
      if styp[i][0] != 'i' :
         if sacc[i] > 0:
            acc.append(scal[i])
      i += 1
   return acc

def RemoveFilesFromFlags(blas, flags):
   words = flags.split()
   nf = ""
   rout= blas + ".b"
   for flag in words:
      if flag.find("-o") == -1 and flag.find(rout) == -1 and \
         flag.find("fkorout.s") == -1:
         nf = nf + flag + " "
   return nf

def RemoveRedundantPrefFlags(flags, pfarrs):
#
#  If we've got a -P all spec, make sure some array not given explicitly
#
   j = flags.find("-P all")
   if j != -1:
      for arr in pfarrs:
         if flags.find("-P %s" % arr) == -1:
            break
      else:
         words = flags[j:].split()
         if j != 0: flags = flags[0:j-1]
         else: flags = ""
         for word in words[4:]:
            flags = flags + " " + word
#
#  Remove repetitive scheduling commands
#
#   j = flags.find("-Ps ")
#   if (j != -1):
#      i = flags[j+3:].find("-Ps ")
#      if (i != -1)
#         words = flags[j:].split()
   return flags
def GetFPInfo(inf):
   na = len(inf[6])
   i=0
   pfarrs = []
   pfsets = []
   pfuses = []
   while(i < na):
      if (inf[7][i] != 0):
         pfarrs.append(inf[6][i])
         pfsets.append(inf[8][i])
         pfuses.append(inf[9][i])
      i += 1
   return(pfarrs, pfsets, pfuses)

def BuildFKO(IFKOdir):
   cmnd = 'cd ' + IFKOdir + '/bin ; make fko'
   fo = fs.popen(cmnd, 'r')
   err = fo.close()
   if (err != None):
      print "command '%s' died with: %d" % (cmnd, err)
      sys.exit(err)

def callfko(fko, flag):
   cmnd = fko + ' ' + flag
#   print cmnd
   fo = os.popen(cmnd, 'r')
   err = fo.close()
   if (err != None):
      print "command '%s' died with: %d" % (cmnd, err)
      sys.exit(err)

def GetStandardFlags(fko, rout, pre):
   inf = info(fko, rout)
   VEC = inf[5]
   LS  = inf[1][0]
   (pfarrs, pfsets, pfuses) = GetFPInfo(inf)
   npf = len(pfarrs)
   if (VEC == 0):
      if (pre == 's'): psiz = 4
      else: psiz = 8
      UR = LS / psiz
      VF = ""
   else:
      UR = LS / 16
      VF = " -V"
   assert(UR > 0)
   KFLAG = VF + " -Ps b A 0 " + str(npf) + " -P all 0 " + str(LS*2) + " -U " \
           + str(UR)
   return KFLAG 

#
# Return relavant opt parameters by reading flag settings
#
def GetOptVals(flags, pfarrs, pfsets):
   if flags.find("-V") == -1 : vec = 0
   else : vec = 1

   j = flags.find("-U ")
   if j == -1: UR = 1
   else:
      words = flags[j:].split()
      UR = int(words[1])

   j = flags.rfind("-Par")
   if j == -1:
      pf0 = "pfnta"
   else :
      words = flags[j+4:].split()
      if words[0][0] == '3':
         pf0 = "pf"
      elif words[0][0] == '0':
         pf0 = "pft0"

   j = flags.find("-Paw")
   pfw = pf0
   if j != -1 :
      words = flags[j+4:].split()
      if words[0][0] == '3':
         pfw = "pfw"
      elif words[0][0] == '0':
         pfw = "pft0"

   pfd = []
   pfl = []
   j = flags.find("-P all")
   if j == -1 :
      for arr in pfarrs:
         pfd.append(0)
         pfl.append(0)
   else :
      words = flags[j:].split();
      lvl = int(words[2])
      dis = int(words[3])
      for arr in pfarrs:
         pfl.append(lvl)
         pfd.append(dis)

   npf = len(pfarrs)
   i = 0
   while i < npf :
      j = flags.find("-P %s" % (pfarrs[i]))
      if j != -1:
         words = flags[j:].split()
         pfl[i] = int(words[2])
         pfd[i] = int(words[3])
      i += 1

   i = 0
   pfinst = []
   while i < npf :
      if pfl[i] > 0 :
         pfinst.append("t%d" % (pfl[i]))
      elif pfl[i] == -1:
         pfinst.append("none")
         pfd[i] = 0
      else :
         if pfsets[i] :
            pfinst.append(pfw)
         else :
            pfinst.append(pf0)
      i += 1

   return(vec, UR, npf, pfinst, pfd)
