import sys

def GetDefaultPre():
    return ['s', 'd']

def GetDefaultKernels():
    return ['swap', 'copy', 'asum', 'axpy', 'dot', 'scal', 'iamax', 'amax', 
            'nrm2','sin','cos','irk1amax', 'irk2amax', 'irk3amax', 'gemvt',
            'gemvn', 'ger1', 'ger2']

def GetAllKernels():
    return ['swap', 'copy', 'asum', 'axpy', 'dot', 'scal', 'iamax', 'amax', 
            'nrm2','sin','cos','irk1amax', 'irk2amax', 'irk3amax', 'gemvt',
            'gemvn', 'ger1', 'ger2']

def GetDefaultBlas():
    return ['swap', 'copy', 'asum', 'axpy', 'dot', 'scal', 'iamax', 'nrm2', 
            'gemvt', 'gemvn', 'ger1', 'ger2']

def GetSVKernels():
    return ['iamax', 'amax', 'nrm2', 'sin', 'cos', 'irk1amax', 
            'irk2amax', 'irk3amax', 'asum']

def GetFKOlevel1Blas():
    return ['swap', 'copy', 'asum', 'axpy', 'dot', 'scal', 'iamax', 'nrm2',
            'amax', 'sin', 'cos', 'irk1amax', 'irk2amax','irk3amax']

def GetLevel1Blas():
    return ['swap', 'copy', 'asum', 'axpy', 'dot', 'scal', 'iamax', 'nrm2']

def GetLevel2Blas():
    return ['gemvt', 'gemvn', 'ger1', 'ger2']

def GetLevel3Blas():
    return ['gemm']

def GetAllFKOGEMM():
    return ['gemm', 'gemm2x2', 'gemm4x1']

def GetBlasPath(blas):
    if blas in GetFKOlevel1Blas():
        return ('/tune/blas/level1/'+blas.upper())
    elif blas == 'gemvt': 
        return ('/tune/blas/gemv/MVTCASES')
    elif blas == 'gemvn': 
        return ('/tune/blas/gemv/MVNCASES')
    elif blas == 'ger1': 
        return ('/tune/blas/ger/R1CASES')
    elif blas == 'ger2': 
        return ('/tune/blas/ger/R2CASES')
    #elif blas == 'gemm': 
    #    return ('/tune/blas/gemm/CASES')
    elif blas in GetAllFKOGEMM(): 
        return ('/tune/blas/gemm/CASES')
    else:
        print "UNKNOWN KERNEL (IN KERNELS)"
        sys.exit(-1);
