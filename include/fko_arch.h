#ifndef FKO_ARCH_H
#define FKO_ARCH_H

/* 
 * for archiregs: 
 * (1) first entry is always stack pointer
 * (2) Second entry is scratch register that can be modified without saving
 *     (for use in prologue)
 */
#if !defined(LINUX_PPC) && !defined(OSX_PPC) && !defined(LINUX_X86_32) && \
    !defined(LINUX_X86_64) && !defined(SOLARIS_SPARC)
/*   #define LINUX_X86_64 */
   #define LINUX_X86_32
/*   #define SOLARIS_SPARC */
/*   #define OSX_PPC */
#endif

#ifdef X86_64
   #define ADDR64
#endif
#if defined(LINUX_X86_32)
   #define X86_32
#endif
#if defined(LINUX_PPC) || defined(OSX_PPC)
   #define PPC
#endif
#if defined(LINUX_X86_64)
   #define X86_64
#endif
#if defined(LINUX_X86_32) || defined(LINUX_X86_64)
   #define X86
#endif
#ifdef SOLARIS_SPARC
   #define SPARC
#endif

#ifdef PPC
   #define ArchHasLoadMul(mul_) ((mul_) == 0 || (mul_) == 1 || (mul_) == -1)
#elif defined(SPARC)
   #define ArchHasLoadMul(mul_) ((mul_) == 0 || (mul_) == 1 || (mul_) == -1)
#elif defined (X86)
   #define ArchHasLoadMul(mul_) \
      ( ((mul_) == 1) || ((mul_) == 2) || ((mul_) == 4) || ((mul_) == 8) || \
        ((mul_) == 0) || ((mul_) == -1) )
   #if defined(X86_64)
      #define ArchPtrIsLong 1
   #endif
   #define ArchConstAndIndex
#endif

#define IREGBEG  1
#define FREGBEG 32
#define DREGBEG 64
/*
 * 1st ireg is always stack pointer.  The next NSIR iregs are registers that
 * can be used by the caller without saving, and the first such register is
 * the register return values of that type are moved to.  The remaining regs
 * may be used, but must be saved in the prologue and restored in epilogue
 * The arch.regs arrays store the actual names of the registers;  In our
 * instructions, register i's name is stored in archiregs[i-1].
 */
#ifdef SPARC
   #define NIR  31                      /* # of integer regs */
   #define TNIR 32
   #define NSIR 15                      /* # of scratch iregs */
   #define NFR  32                      /* # of float regs */
   #define NSFR 32
   #define NDR  32                      /* # of double regs */
   #define NSDR 32
   #define ZEROREG 32
   #define IRETREG 17
   #define FRETREG FREGBEG
   #define DRETREG DREGBEG
   #ifdef ARCH_DECLARE
      int  icallersave[TNIR] = 
       {0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1, 1,1,1,1,1,1,1, 0};
      int  icalleesave[TNIR] = 
       {0, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0, 0,0,0,0,0,0,0, 0};
      char *archiregs[TNIR] = 
         {"@sp", "@l0", "@l1", "@l2", "@l3", "@l4", "@l5", "@l6", "@l7",
          "@i0", "@i1", "@i2", "@i3", "@i4", "@i5", "@i6", "@i7", 
          "@o0", "@o1", "@o2", "@o3", "@o4", "@o5", "@o7",
          "@g1", "@g2", "@g3", "@g4", "g5", "g6", "g7", "@g0"};
      int  fcalleesave[NFR] = 
       {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
      int  fcallersave[NFR] = 
       {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
      char *archfregs[NFR] = 
         {"@f0", "@f1", "@f2", "@f3", "@f4", "@f5", "@f6", "@f7", 
          "@f8", "@f9", "@f10", "@f11", "@f12", "@f13", "@f14", "@f15", 
          "@f16", "@f17", "@f18", "@f19", "@f20", "@f21", "@f22", "@f23", 
          "@f24", "@f25", "@f26", "@f27", "@f28", "@f29", "@f30", "@f31"
         };
      char *archdregs[NDR] = 
         {"@f0", "@f2", "@f4", "@f6", "@f8", "@f10", "@f12", "@f14", 
          "@f16", "@f18", "@f20", "@f22", "@f24", "@f26", "@f28", "@f30", 
          "@f32", "@f34", "@f36", "@f38", "@f40", "@f42", "@f44", "@f46", 
          "@f48", "@f50", "@f52", "@f54", "@f56", "@f58", "@f60", "@f62"
         };
   #else
      extern char *archiregs[TNIR], *archfregs[NFR], *archdregs[NDR];
   #endif
   #define dcallersave fcallersave
   #define dcalleesave fcalleesave
#endif

#ifdef X86_32
   #define NIR 8
   #define NSIR 3
   #define NFR   8                      /* # of float regs */
   #define TNFR  9
   #define NSFR  8
   #define NDR   8                      /* # of double regs */
   #define NSDR  8
   #define IRETREG 4
   #define FRETREG (NFR+FREGBEG)
   #define DRETREG (NFR+DREGBEG)
   #ifdef ARCH_DECLARE
      int icallersave[NIR] = {0, 1, 1, 1, 0, 0, 0, 0};
      int  icalleesave[NIR] = {0, 0, 0, 0, 1, 1, 1, 1};
      char *archiregs[NIR] = 
      {"@esp", "@edx", "@ecx", "@eax", "@ebp", "@ebx", "@esi", "@edi"};
      int  fcallersave[TNFR] = {1,1,1,1,1,1,1,1, 1};
      int  fcalleesave[TNFR] = {0,0,0,0,0,0,0,0, 0};
      char *archfregs[TNFR] = 
      {"@xmm0", "@xmm1", "@xmm2", "@xmm3", "@xmm4", "@xmm5", "@xmm6", "@xmm7",
       "@st"};
   #else
      extern char *archiregs[NIR], *archfregs[TNFR];
   #endif
   #define archdregs archfregs
   #define dcallersave fcallersave
   #define dcalleesave fcalleesave
#endif

#ifdef X86_64
   #define NSR  8
   #define NIR 16
   #define NSIR 3
   #define NFR   16                      /* # of float regs */
   #define NSFR  16 
   #define NDR   16                      /* # of double regs */
   #define NSDR  16 
   #define IRETREG 4
   #define FRETREG FREGBEG
   #define DRETREG DREGBEG
   #ifdef ARCH_DECLARE
      char *archsregs[NSR] = 
      {"@esp", "@edx", "@ecx", "@eax", "@esi", "@edi", "@ebp", "@ebx"};
      int  icallersave[NIR] = {0, 1,1,1,1,1,0,0, 1,1,1,1, 0,0,0,0};
      int  icalleesave[NIR] = {0, 0,0,0,0,0,1,1, 0,0,0,0, 1,1,1,1};
      char *archiregs[NIR] = 
      {"@rsp", "@rdx", "@rcx", "@rax", "@rsi", "@rdi", "@rbp", "@rbx",
       "@r8", "@r9", "@r10", "@r11", "@r12", "@r13", "@r14", "@r15"};
      int  fcallersave[NFR] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
      int  fcalleesave[NFR] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
      char *archfregs[NFR] = 
      {"@xmm0", "@xmm1", "@xmm2", "@xmm3", "@xmm4", "@xmm5", "@xmm6", "@xmm7",
       "@xmm8", "@xmm9", "@xmm10", "@xmm11", "@xmm12", "@xmm13", "@xmm14", 
       "@xmm15"};
   #else
      extern char *archiregs[NIR], *archfregs[NFR], *archsregs[NSR];
   #endif
   #define archdregs archfregs
   #define dcallersave fcallersave
   #define dcalleesave fcalleesave
#endif

#ifdef PPC
   #define NIR  29
   #define TNIR 31
   #define NFR  32
   #define NSFR 18
   #define NDR  32
   #define NSDR 18
   #define IRETREG 3
   #define FRETREG (1+FREGBEG)
   #define DRETREG (1+DREGBEG)
   #ifdef LINUX_PPC
      #define NSIR  11
      #ifdef ARCH_DECLARE
         int  icallersave[TNIR] = 
          {0, 1,1,1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 1}
         int  icalleesave[TNIR] = 
          {0, 0,0,0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 0}
         char *archiregs[TNIR] = 
            {"1", "11", "3", "4", "5", "6", "7", "8", "9", "10", "12",
             "13", "14", "15", "16", "17", "18", "19", "20", "21",
             "22", "23", "24", "25", "26", "27", "28", "29", "30", 
             "31", "0"};
         int  fcallersave[NFR] =
          {1,1,1,1,1,1,1,1,1,1,1,1,1,1 ,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
         int  fcalleesave[NFR] =
          {0,0,0,0,0,0,0,0,0,0,0,0,0,0 ,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
         char *archfregs[NFR] = 
            {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", 
             "11", "12", "13", "14", "15", "16", "17", "18", "19", "20", 
             "21", "22", "23", "24", "25", "26", "27", "28", "29", "30", "31"};
      #else
         extern char *archiregs[TNIR], *archfregs[NFR];
      #endif
   #else
      #define NSIR  10
      #ifdef ARCH_DECLARE
         char *archiregs[TNIR] = 
            {"r1", "r11", "r3", "r4", "r5", "r6", "r7", "r8", "r9","r10","r12",
             "r13", "r14", "r15", "r16", "r17", "r18", "r19", "r20", "r21",
             "r22", "r23", "r24", "r25", "r26", "r27", "r28", "r29", "r30", 
             "r31", "r0"};
         char *archfregs[NFR] = 
            {"f0", "f1", "f2", "f3", "f4", "f5", "f6", "f7", "f8", "f9", "f10", 
             "f11", "f12", "f13", "f14", "f15", "f16", "f17", "f18", "f19", 
             "f20", "f21", "f22", "f23", "f24", "f25", "f26", "f27", "f28", 
             "f29", "f30", "f31"};
      #else
         extern char *archiregs[TNIR], *archfregs[NFR];
      #endif
   #endif
   #define archdregs archfregs
   #define dcallersave fcallersave
   #define dcalleesave fcalleesave
#endif
#ifndef TNIR
   #define TNIR NIR
#endif
#ifndef TNFR
   #define TNFR NFR
#endif
#ifndef TNDR
   #define TNDR NDR
#endif

short GetReg(short type);

#endif
