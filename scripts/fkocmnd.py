import os
import sys
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

def callfko(fko, flag):
   cmnd = fko + ' ' + flag
#   print cmnd
   fo = os.popen(cmnd, 'r')
   err = fo.close()
   if (err != None):
      print "command '%s' died with: %d" % (cmnd, err)
      sys.exit(err)
