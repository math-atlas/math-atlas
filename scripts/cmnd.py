import re
import kernels
import l1cmnd
import l2cmnd
import l3cmnd

def test(ATLdir, ARCH, pre, blas, N, M, lda, rout, cc=None, ccf=None, opt=""):
    
    l1blas = kernels.GetFKOlevel1Blas()
    l2blas = kernels.GetLevel2Blas()
    l3blas = kernels.GetLevel3Blas()
    #fkogemm = kernels.GetAllFKOGEMM()
   
    if blas in l1blas:
        return l1cmnd.test(ATLdir, ARCH, pre, blas, N, rout, cc, ccf, opt)
    elif blas in l2blas:
        return l2cmnd.test(ATLdir, ARCH, pre, blas, N, M, lda, rout, cc, ccf, opt)
#right now, we support gemm kernel with beta = 1, may change the kernel name
# like: gemmkb1 
    elif blas in l3blas:
        return l3cmnd.test(ATLdir, ARCH, pre, blas, N, 1, rout, cc, ccf, opt)
    elif blas in kernels.GetAllFKOGEMM():
        return l3cmnd.test(ATLdir, ARCH, pre, blas, N, 1, rout, cc, ccf, opt)
    else:
        print "UNKNOWN KERNEL [FOR CMND] !!! "
        return -1

def time(ATLdir, ARCH, pre, blas, N, M, lda, rout, cc=None, ccf=None, opt=""):
    
    l1blas = kernels.GetFKOlevel1Blas()
    l2blas = kernels.GetLevel2Blas()
    l3blas = kernels.GetLevel3Blas()
   
    if blas in l1blas:
        return l1cmnd.time(ATLdir, ARCH, pre, blas, N, rout, cc, ccf, opt)
    elif blas in l2blas:
        return l2cmnd.time(ATLdir, ARCH, pre, blas, N, M, lda, rout, cc, ccf, opt)
    elif blas in l3blas:
        return l3cmnd.time(ATLdir, ARCH, pre, blas, N, 1, rout, cc, ccf, opt)
    elif blas in kernels.GetAllFKOGEMM():
        return l3cmnd.time(ATLdir, ARCH, pre, blas, N, 1, rout, cc, ccf, opt)
    else:
        print "UNKNOWN KERNEL [FOR CMND] !!! "
        return [-1, -1]
