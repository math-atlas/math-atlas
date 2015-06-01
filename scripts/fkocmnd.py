import os
import sys
import re

def GetFKOinfo():
   pwd = os.getcwd()
   j = pwd.rfind('iFKO')
   IFKOdir = pwd[0:j+4]
   fko = IFKOdir + '/bin/fko'
   return(IFKOdir, fko)

def FindAtlas(FKOdir):
   file = os.path.join(FKOdir, 'scripts')
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

def GetArchInfo(fko):
    """ i'll only return the cache info frist to make the existing script 
    runnable, will parse the whole file later
    """
#
#  getting the optloop info by using -i fko argument
#
    cmnd = fko + ' -iarch stdout ' 
    fi = os.popen(cmnd, 'r')
    lines = fi.readlines()
    err = fi.close()

    ln = 0; # line number
    fpipe = int(lines[ln][lines[ln].find('=')+1:])
    if (fpipe !=0):
        ln += fpipe
    ln += 1
    #print lines[ln]
    regt = int(lines[ln][lines[ln].find('=')+1:])
    if (regt !=0):
        ln += 2 # skip the num of regs for now
        while (lines[ln].find("ALIASGROUP") != -1):
            ln += 2
    else:
        ln += 1
    #print lines[ln]
    nc = int(lines[ln][8:])
    ln += 1
    words = lines[ln].split()
    #print nc
    #print words
    i = 0
    LS = []
    while (i < nc):
       LS.append(int(words[i+1]))
       i += 1
    #print nc, LS
    return [nc, LS]


#
#  Majedul: this is for new fko's analysis
#
def NewInfo(fko, routine):
   """
   Given the fko path and routine name, this function will run "fko -i" command
   and parse the optloop information for the routine and return the list of 
   parse output. The format of the output list is :
   [nc, LS, ol, maxunroll, lnf, npath, red2onePath, vec, vecMethod, \
           vpathinfo, arrs, pref, sets, uses, scalinf]
   """

#
#  getting the optloop info by using -i fko argument
#
   cmnd = fko + ' -i stdout ' +routine
   fi = os.popen(cmnd, 'r')
   lines = fi.readlines()
   err = fi.close()
#
#  checking for read error
#
   if (err != None):
      print 'command died with: %d' % (err)
      sys.exit(err)
#
#  find the cache info 
#
#   nc = int(lines[0][8:])
#   words = lines[1].split()
#   #print nc
#   i = 0
#   LS = []
#   while (i < nc):
#      LS.append(int(words[i+2]))
#      i += 1
    
   [nc, LS] = GetArchInfo(fko)
   lineNum = 2
#
#  initialize  all the data returns ...
#
   red2onePath = []
   vecMethod = []
   vpathinfo = []
   scalinf = (0, [], [], [], [], [])
   arrs = []
   arrtypes = []
   pref = []
   sets = []
   uses = []
   arr_ur = []
   vec = 0
   nifs = 0
   #ol = int(lines[2][8:])
   lineNum = 0
   ol = int(lines[lineNum][lines[lineNum].find('=')+1:])
   lineNum += 1
#
#  If it has an optloop to perform optimization
#  line 3, 4 , 4. lineNum is 3 here
#
   if (ol != 0):
      maxunroll = int(lines[lineNum][lines[lineNum].find('=')+1:])
      lnf = int(lines[lineNum+1][lines[lineNum+1].find('=')+1:])
      npath = int(lines[lineNum+2][lines[lineNum+2].find('=')+1:])
      lineNum += 3
#
#        number of if statements
#
      nifs = int(lines[lineNum][lines[lineNum].find('=')+1:])
      lineNum += 1
#
#     has multiple loop paths?
#
      if (npath > 1):
#
#        update info for reducible methods of paths        
#        standard code for this: 
#        -----------------------
#         for i in range(lineNum,lineNum+4):  # line 10 for vec, omit it 
#            red2onePath.append(int(lines[i][lines[i].find('=')+1:]))
#
#        using the syntax of list comprehension 
#
         #print lines[lineNum]
         red2onePath = [int(lines[i][lines[i].find('=')+1:]) \
                        for i in range(lineNum, lineNum+3)]
         #print red2onePath
         vec = int(lines[lineNum+3][lines[lineNum+3].find('=')+1:])
         lineNum += 4
#        
#        update vectorization method 
#
#         for i in range(lineNum,lineNum+5):
#            vecMethod.append(int(lines[i][lines[i].find('=')+1:]))
#
#        syntax of list comprehension
#
         vecMethod = [int(lines[i][lines[i].find('=')+1:]) \
                      for i in range(lineNum, lineNum+3)]
         vspec = vecMethod[-1] # -1 indicate the last eliment 
         lineNum += 3
#
#        if Vspec can be applied, findout the path info for vec
#
         if (vspec != 0):
            i = 0
            while (i < npath):
               vpathinfo.append(int(lines[lineNum][lines[lineNum].find('=')+1:]))
               lineNum += 1
               i += 1;
      else: # npath = 1
         vec = int(lines[lineNum][lines[lineNum].find('=')+1:])
         lineNum += 1
#
#     info about moving floating pointer 
#
      mfp = int(lines[lineNum][lines[lineNum].find(':')+1:])
      lineNum += 1
      i = 0;
      while (i < mfp):
         words = lines[lineNum].split()
         k = len(words)
         j = len(words[0])
         arrs.append(words[0][1:j-2])
         j = words[1].rfind('=')
         arrtypes.append(words[1][j+1:])
         j = words[2].rfind("=")
         pref.append(int(words[2][j+1:]))
         j = words[3].rfind("=")
         sets.append(int(words[3][j+1:]))
         j = words[4].rfind("=")
         uses.append(int(words[4][j+1:]))
#
#        if array dim == 2, save the unroll factor   
#
         if k > 5 : # word[5] arr=2d
             j = words[6].rfind("=")
             arr_ur.insert(i, int(words[6][j+1:]))
#
#            need to scope out for nr (ptr+ldas)
#
         else:
             arr_ur.insert(i, 1)
         lineNum += 1
         i += 1;
      #print arr_ur
#
#     info about scalars 
#
      ns = int(lines[lineNum][lines[lineNum].find(':')+1:])
      lineNum += 1
      scal = []
      styp = []
      sset = []
      suse = []
      se = []
      i = 0
      while i < ns:
         words = lines[lineNum].split()
         j = len(words[0])
         scal.append(words[0][1:j-2])
         j = words[1].rfind("=")
         styp.append(words[1][j+1])
         j = words[2].rfind("=")
         sset.append(int(words[2][j+1:]))
         j = words[3].rfind("=")
         suse.append(int(words[3][j+1:]))
         j = words[4].rfind("=")
         se.append(int(words[4][j+1:]))
         lineNum += 1
         i += 1
      scalinf = (ns, scal, styp, sset, suse, se)

   else: #no optloop 
      maxunroll = lnf = vec = mfp = npath = 0
#
#  returning the whole list of info, maintain previous order to keep backward
#  compatibilty
#
   #print pref
   return [nc, LS, ol, maxunroll, lnf, vec, arrs, pref, sets, uses, scalinf, \
           npath, red2onePath, vecMethod, vpathinfo, arrtypes, arr_ur]

#
# returns info from fko's analysis using -i
# Majedul: this is the old version and now obsolete
#
def info(fko, routine):
   cmnd = fko + ' -i stdout ' + routine
#   print cmnd
   fi = os.popen(cmnd, 'r')
   lines = fi.readlines()
   err = fi.close()
   if (err != None):
      print 'command died with: %d' % (err)
      print "cmnd='%s'" % (cmnd)
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

def GetSchPF(npf, pfurs):
    schpf = npf
    i = 0
    while (i < npf):
        schpf = schpf + pfurs[i] - 1
        i = i + 1
    return schpf


def SetDefaultPFD(KFLAG0, inf):
#
#  Remove all prefetch flags
#
   KFLAG = KFLAG0

   matches= re.findall(r'-Ps\s\w\s\w\s\d+\s\d+',KFLAG0) ## find -Ps b A 0 1
   for ps in matches:
      KFLAG = KFLAG.replace(ps, '') ## remove the pattern
   
   matches = re.findall(r'-P\s\w+\s[-]?\d+\s\d+',KFLAG0) ## find -P X 0 1
   for pfs in matches:
      KFLAG = KFLAG.replace(pfs, '') ## remove the pattern
#
#  add default prefetch flags
#
   LS  = inf[1][0]
   (pfarrs, pfsets, pfuses, pfurs) = GetFPInfo(inf)
#
#   if we have 2D array, we will look into the unroll factor
#
   npf = len(pfarrs)
   schpf = GetSchPF(npf, pfurs)
   #KFLAG = KFLAG + " -Ps b A 0 " + str(npf) + " -P all 0 " + str(LS*2) 
   KFLAG = KFLAG + " -Ps b A 0 " + str(schpf) + " -P all 0 " + str(LS*2) 
   return KFLAG

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
   pfurs = []
   #print inf
   #print na
   while(i < na):
      if (inf[7][i] != 0):
         pfarrs.append(inf[6][i])
         pfsets.append(inf[8][i])
         pfuses.append(inf[9][i])
         pfurs.append(inf[16][i])
      i += 1
   return(pfarrs, pfsets, pfuses, pfurs)

def BuildFKO(IFKOdir):
   cmnd = 'cd ' + IFKOdir + '/bin ; make fko'
   fo = fs.popen(cmnd, 'r')
   err = fo.close()
   if (err != None):
      print "command '%s' died with: %d" % (cmnd, err)
      sys.exit(err)

def callfko(fko, flag):
   cmnd = fko + ' ' + flag
   #print cmnd
   fo = os.popen(cmnd, 'r')
   err = fo.close()
   if (err != None):
      print "command '%s' died with: %d" % (cmnd, err)
      sys.exit(err)
#
#  parameterize standard flag for Vec and SB
#
def GetOptStdFlags(fko, rout, pre, VEC, SB=0, URF=0):
   newinfo = NewInfo(fko, rout)
   inf = [newinfo[i] for i in range(11) ]
   #inf = newinfo
   vec = inf[5]
   if vec and VEC:
      VEC = 1
   else:
      VEC = 0
   LS  = inf[1][0]
   #(pfarrs, pfsets, pfuses, pfurs) = GetFPInfo(inf)
   (pfarrs, pfsets, pfuses, pfurs) = GetFPInfo(newinfo)
   npf = len(pfarrs)
#
#   consider unroll factor of 2d array to schedule pref inst
#
   schpf = GetSchPF(npf, pfurs)
   #print schpf
   if (VEC == 0):
      if (pre == 's'): psiz = 4
      else: psiz = 8
#
#     If URF > 0, force Unroll to URF
#
      if URF:
         UR = URF
      else:
         UR = LS / psiz
      UF = " -U %d" %UR
      VF = ""
   else: ##  VEC
      if URF:
         UR = URF
         UF = " -U %d" %UR
      else: ## no force UR
         if SB:
            UR = int(LS / (32*SB))      ## for AVX 
            if UR < 1:
               UF = " -U %d" %UR
            else:
               UF = ""
         else: ## no SB
            UR = LS / 32      ## for AVX 
            UF = " -U %d" %UR
      VF = " -V"
   #KFLAG = VF + " -Ps b A 0 " + str(npf) + " -P all 0 " + str(LS*2) + UF
   KFLAG = VF + " -Ps b A 0 " + str(schpf) + " -P all 0 " + str(LS*2) + UF
   return KFLAG 

def GetStandardFlags(fko, rout, pre):
   #inf = info(fko, rout)
   newinfo = NewInfo(fko, rout)
   inf = [newinfo[i] for i in range(11) ]
   #inf = newinfo

   VEC = inf[5]
   LS  = inf[1][0]
   #(pfarrs, pfsets, pfuses, pfurs) = GetFPInfo(inf)
   (pfarrs, pfsets, pfuses, pfurs) = GetFPInfo(newinfo)
   npf = len(pfarrs)
   if (VEC == 0):
      if (pre == 's'): psiz = 4
      else: psiz = 8
      UR = LS / psiz
      VF = ""
   else:
      #UR = LS / 16
      UR = LS / 32      ## for AVX 
      VF = " -V"
   assert(UR > 0)
   #KFLAG = VF + " -Ps b A 0 " + str(npf) + " -P all 0 " + str(LS*2) + " -U " \
   #        + str(UR)
   schpf = GetSchPF(npf, pfurs)
   KFLAG = VF + " -Ps b A 0 " + str(schpf) + " -P all 0 " + str(LS*2) + " -U " \
           + str(UR)
   return KFLAG 

#
# Return relavant opt parameters by reading flag settings
#
def GetOptVals(flags, pfarrs, pfsets, accs):
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
#
#  Find AE targs
#
   aes = []
   for acc in accs :
      j = flags.find("-AE %s" % acc)
      if j != -1 :
         words = flags[j:].split()
         aes.append(int(words[2]))
      else : aes.append(0)
#
#  Find write-thru flags: -W <name>
#
   WT = []
   j = flags.find(" -W ")
   while j != -1 :
      ln = flags[j+4:]
      words = ln.split()
      WT.append(words[0])
      j = ln[j+4:].find(" -W ")

   return(vec, UR, npf, pfinst, pfd, aes, WT)
