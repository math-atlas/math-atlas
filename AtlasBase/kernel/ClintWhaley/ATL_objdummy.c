If you are looking at this thinking it is the source of your kernel, you
need to scope the ATLAS/tune/blas/gemm/objs directory instead.  This dummy
file is used when ATLAS is actually using a precompiled binary for the
kernel.  Scope the MMFLAGS of used to compile this routine to find out
what object file from objs is being used.
