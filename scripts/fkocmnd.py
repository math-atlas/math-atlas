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
         i += 1;
   else:
      maxunroll = lnf = vec = mfp = 0
   return(nc, LS, ol, maxunroll, lnf, vec, arrs, pref, sets)

def GetPFInfo(inf):
   na = len(inf[6])
   i=0
   pfarrs = []
   pfsets = []
   while(i < na):
      if (inf[7][i] != 0):
         pfarrs.append(inf[6][i])
         pfsets.append(inf[8][i])
      i += 1
   return(pfarrs, pfsets)

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
   (pfarrs, pfsets) = GetPFInfo(inf)
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
