#ifndef FKO_INST_H
#define FKO_INST_H

enum comp_flag
/*
 * enumerated type for compiler flag, so format is:
 * CMPFLAG, <enum comp_flag>, <info1>, <info2>
 */
{
   CF_REGSAVE,
   CF_REGRESTORE,
   CF_PARASAVE,
   CF_LOOP_INIT,  /* loop# */
   CF_LOOP_BODY,
   CF_LOOP_UPDATE,
   CF_LOOP_PTRUPDATE,
   CF_LOOP_TEST,
   CF_LOOP_END,
   CF_VRED_END,
   CF_SCAL_RES,     /* for Speculative vectorization*/
   CF_SSV_SUPDATE,
   CF_SSV_VUPDATE,
   CF_SSV_VBACKUP,
   CF_SSV_VBACKUP_END,
   CF_SSV_VRECOVERY,
   CF_FORCE_ALIGN
};

/*
 *
 * [r]  = integer register
 * [fr] = fp register
 * [c]  = constant
 * [cc] = condition code
 *
 * Unless otherwise noted, all instructions have format:
 * [r0], [r1], [r2/c2]
 * NOTE: only use 1st 14 bits, 2 most sig used to encode exceptions for
 *       instructions that update more than dest:
 *       00 : normal (dest updated only)
 *       01 : dest & src1 updated
 *       10 : dest & src2 updated
 *       11 : all ops updated
 */
enum inst
{
   UNIMP,                       /* NULL, NULL, NULL */
   NOP,                         /* NULL, NULL, NULL */
   COMMENT,                     /* <string>,NULL,NULL */
   LABEL,                       /* <labname>,NULL,NULL */
   CMPFLAG,                     /* flag,flag,flag compiler info */
/*
 * integer ops of same size as ptrs
 */
   LD,                          /* [r], [ptr], NULL */
   ST,                          /* [ptr], [r], NULL */
   MOV,                         /* [r0], [r1/c] : r0 = r1 */
   OR,                          /* r0 = r1 | r/c */
   AND,                         /* r0 = r1 & r/c */
   ANDCC,                       /* r0 = r1 & r/c */
   XOR,                         /* r0 = r1 ^ r/c */
   NOT,                         /* r0 = ~r0 */
   SHL,                         /* r0 = r1 << r/c */
   SHLCC,                       /* r0 = r1 << r/c */
   SHR,                         /* r0 = r1 >> r/c, set [cc] */
   SAR,                         /* r0 = r1 >> r/c */
   ADD,                         /* r0 = r1 + r/c */
   SUB,                         /* r0 = r1 - r/c */
   SUBCC,                       /* r0 = r1 - r/c */
   MUL,                         /* r0 = r1 * r/c */
   UMUL,                        /* r0 = r1 * r/c, unsigned */
   DIV,                         /* r0 = r1 / r/c */
   UDIV,                      
   CMPAND,                      /* cc0, r1, r2/c : set [cc] based on r1 & r2 */
   CMP,                         /* cc#, r1, r2/c: set [cc] based on r1 - r2 */
   NEG,                         /* [r0], [r1] : r0 = -r1 */
   LEA2,                        /* lea r0, [r1, r2, 2] */ 
   LEA4,                        /* lea r0, [r1, r2, 4] */ 
   LEA8,                        /* lea r0, [r1, r2, 8] */ 
/*   ABS, ; abs commented out because not widely supported */
/*
 * 32-bit integer (64-bit systems only)
 */
   LDS,                         /* [r], [ptr], NULL */
   STS,                         /* [ptr], [r], NULL */
   ORS,                         /* r0 = r1 | r/c */
   XORS,                        /* r0 = r1 ^ r/c */
   NOTS,                        /* r0 = ~r0 */
   SHLS, SHLCCS,                /* r0 = r1 << r/c */
   SHRS, SHRCCS,                /* r0 = r1 >> r/c, set [cc] */
   SARS,                        /* r0 = r1 >> r/c */
   ADDS, ADDCCS,                /* r0 = r1 + r/c */
   SUBS, SUBCCS,                /* r0 = r1 - r/c */
   MULS,                        /* r0 = r1 * r/c */
   UMULS,                       /* r0 = r1 * r/c, unsigned */
   DIVS,                        /* r0 = r1 / r/c */
   UDIVS,
   CMPS,                        /* set [cc](r0) based on r1 - r2 */
   MOVS,                        /* [r0], [r1] : r0 = r1 */
   NEGS,                        /* [r0], [r1] : r0 = -r1 */
   LEAS2,                        /* lea r0, [r1, r2, 2] */ 
   LEAS4,                        /* lea r0, [r1, r2, 4] */ 
   LEAS8,                        /* lea r0, [r1, r2, 8] */ 
   /*ABSS,*/
/*
 * Jump instructions
 */
   JMP,                         /* PCREG, LABEL, 0 */
   JEQ, JNE, JLT, JLE, JGT, JGE,  /* PCREG, cc#, LABEL */
   RET,                         /* NULL,NULL,NULL : return to caller */
   PREFR,                       /* NULL, [ptr] [ilvl] */
   PREFW,                       /* NULL, [ptr] [ilvl] */
/*
 * The stream prefetch instructions have format:
 * [ptr], [len], [ilvl:ist]
 * where [ilvl:ist] is an 8-bit constant, where 4 most sig bits give cache
 * level to fetch to (if leading bit is set, fetch is inclusive, else 
 * exclusive).  The last 4 bits indicate which stream to fetch with.
 * Therefore, up to 7 cache levels and 15 streams allowed.
 */
   PREFRS,                      /* [ptr], [len], [ilvl:ist] */
   PREFWS,                      /* [ptr], [len], [ilvl:ist] */
/*
 * Floating point instructions, single precision
 */
   FZERO,                       /* [fr] : fr = 0.0 */
   FLD,                         /* [fr] [ptr] */
   FST,                         /* [ptr], [fr] */
   FMAC,                        /* fr0 += fr1 * fr2 */
   FMUL,                        /* fr0 = fr1 * fr2 */
   FDIV,                        /* fr0 = fr1 / fr2 */
   FADD,                        /* fr0 = fr1 + fr2 */
   FSUB,                        /* fr0 = fr1 - fr2 */
   FABS,                        /* [fr0], [fr1] : fr0 = abs(fr1) */
   FCMP,                        /* [fr0], [fr1] : set [cc] by (fr0 - fr1) */
   FNEG,                        /* fr0, fr1 : fr0 = -fr1 */
   FMAX,                        /* fr0 = max(fr1,fr2) */
   FMIN,                        /* fr0 = min(fr1,fr2) */
   FMOV,                        /* fr0 = fr1 */
/*
 * Conditional move
 */
   FCMOV1,                        /* fr0 = ireg? fr0 : fr1/mem */
   FCMOV2,                        /* fr0 = ireg? fr1/mem : fr0 */
/*
 * Floating point instructions, double precision
 */
   FZEROD,                      /* [fr] : fr = 0.0 */
   FLDD,                        /* [fr] [ptr] */
   FSTD,                        /* [ptr], [fr] */
   FMACD,                       /* fr0 += fr1 * fr2 */
   FMULD,                       /* fr0 = fr1 * fr2 */
   FDIVD,                       /* fr0 = fr1 / fr2 */
   FADDD,                       /* fr0 = fr1 + fr2 */
   FSUBD,                       /* fr0 = fr1 - fr2 */
   FABSD,                       /* [fr0], [fr1] : fr0 = abs(fr1) */
   FCMPD,                       /* [fr0], [fr1] : set [cc] by (fr0 - fr1) */
   FNEGD,                       /* [fr0], [fr1] : fr0 = -fr1 */
   FMAXD,                       /* fr0 = max(fr1,fr2) */
   FMIND,                       /* fr0 = min(fr1,fr2) */
   FMOVD,                       /* fr0 = fr1 */
/*
 * Conditional Move
 */
   FCMOVD1,                        /* fr0 = ireg? fr0 : fr1 */
   FCMOVD2,                        /* fr0 = ireg? fr1 : fr0 */
/*
 * special instruction to support FABS and FNEG in x86 using VAND/VXOR  
 * it's ugly but x86 doesn't have andss/xorss inst
 * So, these are hybrid instructions which has both scalar and vector
 * operands
 */
   VFSABS,                        /* fr0 = fr1 & vfr2/mem */
   VFSNEG,                        /* fr0 = fr1 xor vfr2/mem */ 
   VDSABS,                        /* fr0 = fr1 & vfr2/mem */
   VDSNEG,                        /* fr0 = fr1 xor vfr2/mem */ 
/*
 * Double precision vector instructions
 * [memA] is a vector-aligned mem @ [mem] is any alignment
 */
   VDZERO,                     /* [vr0]        : vr0[0:N] = 0.0 */
   VDLD,                       /* [vr0], [memA]  : vr0 = mem */
   VDLDU,                      /* [vr0], [memU]  : vr0 = unaligned mem */
   VDLDS,                      /* [vr0], [mem]  :  vr0[0] = mem; vr0[1] = 0 */
   VDLDL,                      /* [vr], [mem]   : vr[0] = mem; vr[1] = vr[1] */
   VDLDH,                      /* [vr], [mem]   : vr[0] = vr[0]; vr[1] = mem */
   VDSTNT,                     /* [memA], vr0    : mem = vr0 */
   VDST,                       /* [memA], vr0    : mem = vr0 */
   VDSTU,                      /* [memU], vr0    : unaligned mem = vr0 */
   VDSTS,                      /* [mem], [vr0]  :  mem = vr[0] */   
   VDMOV,                      /* [vr0], [vr1]   : vr0 = vr1 */
/*
 * Majedul: made v[i,f,d]movs 3 operands operation too.
 *  1. VDMOVS vr0, r0, vr1  // vr0[0] = r0, vr0[vlen-1: 1] = vr1[vlen-1: 1]
 *  2. VDMOVS r0, vr0, 0    // r0 = vr0[0]  
 */
   VDMOVS,                     /* [vr0], [vr1], [vr2] */
   VDADD,                      /* [vr0], [vr1], [vr2] : vr0 = vr1 + vr2 */
   VDSUB,                      /* [vr0], [vr1], [vr2] : vr0 = vr1 - vr2 */
   VDMUL,                      /* [vr0], [vr1], [vr2] : vr0 = vr1 * vr2 */
   VDDIV,                      /* [vr0], [vr1], [vr2] : vr0 = vr1 * vr2 */ 
   VDMAC,                      /* [vr0], [vr1], [vr2] : vr0 += vr1 * vr2 */
   VDABS,                      /* [vr0], [vr1] : vr0 = abs(vr1) */
   VDNEG,                      /* [vr0], [vr1] : vr0 = -(vr1) */
   VDMAX,                      /* vr0 = max(vr1,vr2) */
   VDMIN,                      /* vr0 = min(vr1,vr2) */
   VDHADD,                     /* vr0 = HADD(vr1), HADD(vr2)*/
/*
 * Majedul: intorducing special load instruction with broadcast ability
 * NOTE: it will only be used to load data. repeatble optimizations skip it
 * it doesn't have corresponding move instruction to optimize
 */
   VDLDSB,                     /* vr, mem : vr[0] = vr[1] = ...= [mem] */
/*
 * Majedul: 
 * NOTE: SSE -> 3 operand, AVX -> 4 operand
 */
   VDCMOV1,                      /* vr0 = vr2? vr0 : vr1 */ 
   VDCMOV2,                      /* vr0 = vr2? vr1 : vr0 */ 

   VDSHUF,                     /* [vr0], [vr1], [int32]; vr0 = shuf(vr1|vr0) */
          /* [int32] is split into 8 4 bit words; 1st word indicates which */
          /* should reside in vr0[0], 4th in vr0[3];  Words are numbered */
          /* starting in vr0[0], and ending in vr1[N], N=veclen-1 */
   VDHIHALF,                  /* vr0~2[1,0]: vr0[0] = vr1[1], vr0[1] = vr2[1] */
   VDLOHALF,                  /* vr0~2[1,0]: vr0[0] = vr1[0], vr0[1] = vr2[0] */
/*
 * Single precision vector instructions
 * [memA] is a vector-aligned mem @ [mem] is any alignment
 */
   VFZERO,                     /* [vr0]        : vr0[0:N] = 0.0 */
   VFLD,                       /* [vr0], [memA]  : vr0 = mem */
   VFLDU,                      /* [vr0], [memU]  : vr0 = unaligned mem */
   VFLDS,                      /* [vr0], [mem]  :  vr0[0] = mem; vr0[1] = 0 */
   VFLDL,                      /* [vr], [mem]   : vr[0] = mem; vr[1] = vr[1] */
   VFLDH,                      /* [vr], [mem]   : vr[0] = vr[0]; vr[1] = mem */
   VFSTNT,                     /* [memA], vr0    : mem = vr0; no cache read */
   VFST,                       /* [memA], vr0    : mem = vr0 */
   VFSTU,                      /* [memU], vr0    : unaligned mem = vr0 */
   VFSTS,                      /* [mem], [vr0]  :  mem = vr[0] */
   VFMOV,                      /* [vr0], [vr1]   : vr0 = vr1 */
/*
 * Majedul: made v[i,f,d]movs 3 operands operation too.
 *  1. VFMOVS vr0, r0, vr1  // vr0[0] = r0, vr0[vlen-1: 1] = vr1[vlen-1: 1]
 *  2. VFMOVS r0, vr0, 0    // r0 = vr0[0]  
 */
   VFMOVS,                     /* [vr0], [vr1], [vr2] */
   VFADD,                      /* [vr0], [vr1], [vr2] : vr0 = vr1 + vr2 */
   VFSUB,                      /* [vr0], [vr1], [vr2] : vr0 = vr1 - vr2 */
   VFMUL,                      /* [vr0], [vr1], [vr2] : vr0 = vr1 * vr2 */
   VFDIV,                      /* [vr0], [vr1], [vr2] : vr0 = vr1 * vr2 */
   VFMAC,                      /* [vr0], [vr1], [vr2] : vr0 += vr1 * vr2 */
   VFABS,                      /* [vr0], [vr1] : vr0 = abs(vr1) */
   VFNEG,                      /* [vr0], [vr1] : vr0 = -(vr1) */
   VFMAX,                      /* vr0 = max(vr1,vr2) */
   VFMIN,                      /* vr0 = min(vr1,vr2) */
   VFHADD,                     /* vr0 = HADD(vr1), HADD(vr2)*/
/*
 * Majedul: intorducing special load instruction with broadcast ability
 * NOTE: it will only be used to load data. repeatble optimizations skip it
 */
   VFLDSB,                     /* vr, mem: vr[0] = vr[1] =...=[mem] */
/*
 * Majedul: 
 * NOTE: SSE -> 3 operand, AVX -> 4 operand
 * ---------
 */
   VFCMOV1,                      /* vr0 = vr2? vr0 : vr1 */
   VFCMOV2,                      /* vr0 = vr2? vr1 : vr0 */

   VFSHUF,                     /* [vr0], [vr1], [int32]; vr0 = shuf(vr1|vr0) */
          /* [int32] is split into 8 4 bit words; 1st word indicates which */
          /* should reside in vr0[0], 4th in vr0[3];  Words are numbered */
          /* starting in vr0[0], and ending in vr1[N], N=veclen-1 */
   VFHIHALF,                  /* vr0~2[1,0]: vr0[0] = vr1[1], vr0[1] = vr2[1] */
   VFLOHALF,                  /* vr0~2[1,0]: vr0[0] = vr1[0], vr0[1] = vr2[0] */
/*
 * x86-only instructions
 */
   VGR2VR16,                    /* vreg, ireg, const -- PINSRW */
   FCMPW,    /* freg0, freg1, iconst ; freg0 overwritten with T or F */
   FCMPWD,   /* iconst -- 0 : ==;  1 : < ;  2 : <= */
   CVTBFI,   /* ireg, freg    x86 only movmskps -> bit move (no conversion) */
   CVTBDI,   /* ireg, dreg    x86 only movmskpd */
/*
 * Majedul: some special instruction for Integer. 
 */
#if 0   
   CMOV1,
   CMOV2,
   VCMOV1,
   VCMOV2,
#endif
   CMOV1,
   CMOV2,
/* 32 bit version */   
   VSCMOV1,
   VSCMOV2,
/* 64 bit version */   
   VICMOV1,
   VICMOV2,

/*
 * Majedul: Vector int inst... V_INT type
 * We consider 2 versions: 32 bit and 64 bit integer
 * NOTE: vector loads are same; so, we use VLD instead of VILD and VSLD
 */
/* 32bit or long word version */   
   /*VSMOV,*/
/*
 * Majedul: made v[i,f,d]movs 3 operands operation too.
 *  1. VSMOVS vr0, r0, vr1  // vr0[0] = r0, vr0[vlen-1: 1] = vr1[vlen-1: 1]
 *  2. VSMOVS r0, vr0, 0    // r0 = vr0[0]  
 */
   VSMOVS,
   /*VSLD,*/
   VSLDS,
   /*VSST,*/
   VSSTS,
   VSSHUF,
   VGR2VR32,
   VSADD,
   VSSUB,
   VSMAX,
   VSMIN,
   VSZERO,
/* 64 bit version */
   /*VIMOV,*/
/*
 * Majedul: made v[i,f,d]movs 3 operands operation too.
 *  1. VIMOVS vr0, r0, vr1  // vr0[0] = r0, vr0[vlen-1: 1] = vr1[vlen-1: 1]
 *  2. VIMOVS r0, vr0, 0    // r0 = vr0[0]  
 */
   VIMOVS,
   /*VILD,*/
   VILDS,
   /*VIST,*/
   VISTS,
   VISHUF,
   VGR2VR64,
   VIADD,
   VISUB,
   VIMAX,
   VIMIN,
   VIZERO,
   VICMPWGT,
/* common for 32 and 64 bit */   
   VMOV,
   VLD,                     /* [vr0], [memA]  : vr0 = mem */
   VLDU,                    /* [vr0], [memA]  : vr0 = unaligned mem */
   VST,
   VSTU,                    /* [memU], vr0    : unaligned mem = vr0 */
#if 0   
   VMOV,
   VMOVS,
   VLD,
   VLDS,
   VST,
   VSTS,
   VISHUF,
   VCVTMASKFI,
   VCVTMASKDI,
   MASKTEST,      /* fcc, [fr0]  : set [cc] by (fr0 - 0xFFFF....) */
   VGR2VR32,
   VADD,
   VSUB,
   VMAX,
   VMIN,
   VIZERO,
#endif   
/*
 * special instrcution to update conditional code EFLAGS... 
 */
   BTC,          /* transfer a bit to CF flag:  ICC0, ireg, $position */
/*
 * vector cmp instructions
 */
#if 0   
   VDCMPWEQ,
   VDCMPWNE,
   VDCMPWLT,
   VDCMPWLE,
   VDCMPWNLT,
   VDCMPWNLE,
   VDCMPWGT,
   VDCMPWGE,
   VDCMPWNGT,
   VDCMPWNGE,
   VFCMPWEQ,
   VFCMPWNE,
   VFCMPWLT,
   VFCMPWLE,
   VFCMPWNLT,
   VFCMPWNLE,
   VFCMPWGT,
   VFCMPWGE,
   VFCMPWNGT,
   VFCMPWNGE,
#else
   VDCMPWEQ,
   VDCMPWNE,
   VDCMPWLT,
   VDCMPWLE,
   VDCMPWGT,
   VDCMPWGE,
   VFCMPWEQ,
   VFCMPWNE,
   VFCMPWLT,
   VFCMPWLE,
   VFCMPWGT,
   VFCMPWGE,
#endif
/*
 * Masks instruction: masks sign bits of vector to integer reg
 */
   VFSBTI,
   VDSBTI,
/*
 * Type conversion instructions
 */
   CVTIS,
   CVTSI,
   CVTFI,
   CVTIF,
   CVTDI,
   CVTID,
   CVTFS,
   CVTSF,
   CVTDS,
   CVTSD,
/*
 * Majedul: this is to convert mask into INT
 */
   CVTMASKFI, /* diff CVTBDI ??? */
   CVTMASKDI,
/*
 * Dummy CMP instruction which are needed for ReductionVector translation.
 * I don't find their implementation yet.
 */
#if 0   
   FCMPDWEQ,
   FCMPDWNE,
   FCMPDWLT,
   FCMPDWLE,
   FCMPDWNLT,
   FCMPDWNLE,
   FCMPDWGT,
   FCMPDWGE,
   FCMPDWNGT,
   FCMPDWNGE,
   FCMPWEQ,
   FCMPWNE,
   FCMPWLT,
   FCMPWLE,
   FCMPWNLT,
   FCMPWNLE,
   FCMPWGT,
   FCMPWGE,
   FCMPWNGT,
   FCMPWNGE,
#else
   FCMPDWEQ,
   FCMPDWNE,
   FCMPDWLT,
   FCMPDWLE,
   FCMPDWGT,
   FCMPDWGE,
   FCMPWEQ,
   FCMPWNE,
   FCMPWLT,
   FCMPWLE,
   FCMPWGT,
   FCMPWGE,
#endif
   LAST_INST
};

#ifdef IFKO_DECLARE
char *instmnem[] =
{
   "UNIMP",
   "NOP",
   "COMMENT",
   "LABEL",
   "CMPFLAG",
/*
 * integer ops of same size as ptrs
 */
   "LD",
   "ST",
   "MOV",
   "OR",       /* [OR-NEG] set CC for x86 */
   "AND",
   "ANDCC",
   "XOR",
   "NOT",
   "SHL",
   "SHLCC",
   "SHR",
   "SAR",
   "ADD",
   "SUB",
   "SUBCC",
   "MUL",
   "UMUL",
   "DIV",
   "UDIV",
   "CMPAND",
   "CMP",
   "NEG",
   "LEA2",
   "LEA4",
   "LEA8",
/*   ABS, ; abs commented out because not widely supported */
/*
 * 32-bit integer (64-bit systems only)
 */
   "LDS",
   "STS",
   "MOVS",
   "ORS",
   "XORS",
   "NOTS",
   "SHLS", "SHLCCS",
   "SHRS", "SHRCCS",
   "SARS",
   "ADDS", "ADDCCS",
   "SUBS", "SUBCCS",
   "MULS",
   "UMULS",
   "DIVS",
   "UDIVS",
   "CMPS",
   "NEGS",
   "LEAS2",
   "LEAS4",
   "LEAS8", 
   /*"ABSS",*/
/*
 * Jump instructions
 */
   "JMP",
   "JEQ", "JNE", "JLT", "JLE", "JGT", "JGE",
   "RET",
   "PREFR",
   "PREFW",
/*
 * The stream prefetch instructions have format:
 * [ptr], [len], [ilvl:ist]
 * where [ilvl:ist] is an 8-bit constant, where 4 most sig bits give cache
 * level to fetch to (if leading bit is set, fetch is inclusive, else 
 * exclusive).  The last 4 bits indicate which stream to fetch with.
 * Therefore, up to 7 cache levels and 15 streams allowed.
 */
   "PREFRS",
   "PREFWS",
/*
 * Floating point instructions, single precision
 */
   "FZERO",
   "FLD",
   "FST",
   "FMAC",
   "FMUL",
   "FDIV",
   "FADD",
   "FSUB",
   "FABS",
   "FCMP",
   "FNEG",
   "FMAX",
   "FMIN",
   "FMOV",
   "FCMOV1",
   "FCMOV2",
/*
 * Floating point instructions, double precision
 */
   "FZEROD",
   "FLDD",
   "FSTD",
   "FMACD",
   "FMULD",
   "FDIVD",
   "FADDD",
   "FSUBD",
   "FABSD",
   "FCMPD",
   "FNEGD",
   "FMAXD",
   "FMIND",
   "FMOVD",
   "FCMOVD1",
   "FCMOVD2",
/*
 * special hybrid inst to support fabs and fneg in x86
 */
   "VFSABS",
   "VFSNEG",
   "VDSABS",
   "VDSNEG",
/*
 * Double precision vector inst
 */
   "VDZERO",
   "VDLD",
   "VDLDU",
   "VDLDS",
   "VDLDL",
   "VDLDH",
   "VDSTNT",
   "VDST",
   "VDSTU",
   "VDSTS",
   "VDMOV",
   "VDMOVS",
   "VDADD",
   "VDSUB",
   "VDMUL",
   "VDDIV",
   "VDMAC",
   "VDABS",
   "VDNEG",
   "VDMAX",
   "VDMIN",
   "VDHADD",
/*
 * Majedul: intorducing special load instruction with broadcast ability
 */
   "VDLDSB",
/*
 * blend operation
 */
   "VDCMOV1",
   "VDCMOV2",
   "VDSHUF",
   "VDHIHALF",
   "VDLOHALF",
/*
 * Single precision vector inst
 */
   "VFZERO",
   "VFLD",
   "VFLDU",
   "VFLDS",
   "VFLDL",
   "VFLDH",
   "VFSTNT",
   "VFST",
   "VFSTU",
   "VFSTS",
   "VFMOV",
   "VFMOVS",
   "VFADD",
   "VFSUB",
   "VFMUL",
   "VFDIV",
   "VFMAC",
   "VFABS",
   "VFNEG",
   "VFMAX",
   "VFMIN",
   "VFHADD",
/*
 * Majedul: intorducing special load instruction with broadcast ability
 */
   "VFLDSB",
/*
 * blend operation
 */
   "VFCMOV1",
   "VFCMOV2",
   "VFSHUF",
   "VFHIHALF",
   "VFLOHALF",
/*
 * x86-only instructions
 */
   "VGR2VR16",
   "FCMPW",
   "FCMPWD",
   "CVTBFI",
   "CVTBDI",
/*
 * Majedul: some special integer instruction
 */
#if 0   
   "CMOV1",
   "CMOV2",
   "VCMOV1",
   "VCMOV2",
#endif
   "CMOV1",
   "CMOV2",
/* 32 bit version */   
   "VSCMOV1",
   "VSCMOV2",
/* 64 bit version */   
   "VICMOV1",
   "VICMOV2",
/*
 * Majedul: instruction for V_INT
 */
/* 32bit or long word version */   
   "VSMOVS",
   /*"VSLD",*/
   "VSLDS",
   /*"VSST",*/
   "VSSTS",
   "VSSHUF",
   "VGR2VR32",
   "VSADD",
   "VSSUB",
   "VSMAX",
   "VSMIN",
   "VSZERO",
/* 64 bit version */
   /*"VIMOV",*/
   "VIMOVS",
   /*"VILD",*/
   "VILDS",
   /*"VIST",*/
   "VISTS",
   "VISHUF",
   "VGR2VR64",
   "VIADD",
   "VISUB",
   "VIMAX",
   "VIMIN",
   "VIZERO",
   "VICMPWGT",
/* common for 64 and 32 bit */   
   "VMOV",
   "VLD",
   "VLDU",
   "VST",
   "VSTU",
#if 0   
   "VMOV",
   "VMOVS",
   "VLD",
   "VLDS",
   "VST",
   "VSTS",
   "VISHUF",
   "VCVTMASKFI",
   "VCVTMASKDI",
   "MASKTEST",
   "VGR2VR32",
   "VADD",
   "VSUB",
   "VMAX",
   "VMIN",
   "VIZERO",
#endif   
/*
 * special instrcution to update conditional code EFLAGS... 
 */
   "BTC",
/*
 * vector cmp intructions
 */
#if 0   
   "VDCMPWEQ",
   "VDCMPWNE",
   "VDCMPWLT",
   "VDCMPWLE",
   "VDCMPWNLT",
   "VDCMPWNLE",
   "VDCMPWGT",
   "VDCMPWGE",
   "VDCMPWNGT",
   "VDCMPWNGE",
   "VFCMPWEQ",
   "VFCMPWNE",
   "VFCMPWLT",
   "VFCMPWLE",
   "VFCMPWNLT",
   "VFCMPWNLE",
   "VFCMPWGT",
   "VFCMPWGE",
   "VFCMPWNGT",
   "VFCMPWNGE",
#else
   "VDCMPWEQ",
   "VDCMPWNE",
   "VDCMPWLT",
   "VDCMPWLE",
   "VDCMPWGT",
   "VDCMPWGE",
   "VFCMPWEQ",
   "VFCMPWNE",
   "VFCMPWLT",
   "VFCMPWLE",
   "VFCMPWGT",
   "VFCMPWGE",
#endif
/*
 * Masks instruction: masks sign bits of vector to integer reg
 */
   "VFSBTI",
   "VDSBTI",
/*
 * Type conversion instructions
 */
   "CVTIS",
   "CVTSI",
   "CVTFI",
   "CVTIF",
   "CVTDI",
   "CVTID",
   "CVTFS",
   "CVTSF",
   "CVTDS",
   "CVTSD",
/*
 * Majedul: experimental... to convert mask from floating point to Int
 */
   "CVTMASKFI",
   "CVTMASKDI",

/*
 * Dummy inst for redundant vector transformation
 */
#if 0   
   "FCMPDWEQ",
   "FCMPDWNE",
   "FCMPDWLT",
   "FCMPDWLE",
   "FCMPDWNLT",
   "FCMPDWNLE",
   "FCMPDWGT",
   "FCMPDWGE",
   "FCMPDWNGT",
   "FCMPDWNGE",
   "FCMPWEQ",
   "FCMPWNE",
   "FCMPWLT",
   "FCMPWLE",
   "FCMPWNLT",
   "FCMPWNLE",
   "FCMPWGT",
   "FCMPWGE",
   "FCMPWNGT",
   "FCMPWNGE",
#else
   "FCMPDWEQ",
   "FCMPDWNE",
   "FCMPDWLT",
   "FCMPDWLE",
   "FCMPDWGT",
   "FCMPDWGE",
   "FCMPWEQ",
   "FCMPWNE",
   "FCMPWLT",
   "FCMPWLE",
   "FCMPWGT",
   "FCMPWGE",
#endif
   "LAST_INST"
};
#else
   extern char *instmnem[];
#endif

#define FIRSTBRANCH JMP
#define LASTBRANCH  RET

#define GET_INST(inst_) ((inst_) & 0x3FFF)
#define ACTIVE_INST(i_) ((i_) != COMMENT && (i_) != CMPFLAG)
#define IS_BRANCH(i_) ((i_) >= FIRSTBRANCH && (i_) <= LASTBRANCH)

/*
 * Majedul: checking conditional branch.
 * assuming, only unconditional branches are JMP and RET and they are set 
 * as 1st and last branches.  
 */

#define IS_COND_BRANCH(i_) ((i_) > FIRSTBRANCH && (i_) < LASTBRANCH)

#define IS_LOAD(i_)  ((i_) == LD || (i_) == FLD || (i_) == FLDD || \
                      (i_) == VFLD || (i_) == VDLD || (i_) == LDS || \
                      (i_) == VFLDS || (i_) == VDLDS || \
                      (i_) == VFLDL || (i_) == VFLDH || \
                      (i_) == VDLDL || (i_) == VDLDH || \
                      (i_) == VLD  ||  (i_) == VSLDS || \
                      (i_) == VILDS || (i_) == VFLDU || \
                      (i_) == VDLDU || (i_) == VLDU  || \
                      (i_) == VFLDSB || (i_) == VDLDSB )

#define IS_UNALIGNED_VLOAD(i_) ((i_) == VFLDU || (i_) == VDLDU || \
                               (i_) == VLDU )
/*
 * Although VFLDSB/VDLDSB loads into vector register, they are essentially 
 * scalar load.
 * NOTE: Should we consider VFLDS/VFSTS as vector or scalar load/store?
 */
#define IS_VLOAD(i_)  ((i_) == VFLD || (i_) == VDLD || \
                      (i_) == VFLDS || (i_) == VDLDS || \
                      (i_) == VFLDL || (i_) == VFLDH || \
                      (i_) == VDLDL || (i_) == VDLDH || \
                      (i_) == VLD  ||  (i_) == VSLDS || \
                      (i_) == VILDS || (i_) == VFLDU || \
                      (i_) == VDLDU || (i_) == VLDU )

#define IS_MOVE(i_) ((i_) == MOV || (i_) == FMOV || (i_) == FMOVD || \
                     (i_) == VFMOV || (i_) == VDMOV || (i_) == VMOV || \
                     (i_) == VFMOVS || (i_) == VDMOVS || (i_) == VIMOVS ||\
                     (i_) == VSMOVS )

#define IS_MAC(i_) ((i_) == FMAC || (i_) == FMACD || (i_) == VFMAC || \
                    (i_) == VDMAC)
#define IS_STORE(i_)  ((i_) == ST || (i_) == FST || (i_) == FSTD || \
                       (i_) == VFST || (i_) == VDST || (i_) == STS || \
                       (i_) == VFSTS || (i_) == VDSTS || \
                       (i_) == VFSTNT || (i_) == VDSTNT || \
                       (i_) == VST || (i_) == VSSTS || \
                       (i_) == VISTS || (i_) == VFSTU || \
                       (i_) == VDSTU || (i_) == VSTU )

#define IS_VSTORE(i_)  ((i_) == VFST || (i_) == VDST || \
                       (i_) == VFSTS || (i_) == VDSTS || \
                       (i_) == VFSTNT || (i_) == VDSTNT || \
                       (i_) == VST || (i_) == VSSTS || \
                       (i_) == VISTS || (i_) == VFSTU || \
                       (i_) == VDSTU || (i_) == VSTU )

#define IS_V2SST(i_)  ((i_) == VFSTS || (i_) == VDSTS || \
                       (i_) == VISTS || (i_) == VSSTS)

#define IS_UNALIGNED_VSTORE(i_) ((i_) == VFSTU || (i_) == VDSTU || \
                                 (i_) == VSTU )

#define IS_CMPW(i_)  ((i_) == FCMPDWEQ || (i_) == FCMPDWNE || \
                      (i_) == FCMPDWLT || (i_) == FCMPDWLE || \
                      (i_) == FCMPDWGT || (i_) ==  FCMPDWGE || \
                      (i_) == FCMPWEQ || (i_) ==  FCMPDWNE || \
                      (i_) == FCMPWLT || (i_) ==  FCMPWLE || \
                      (i_) == FCMPWGT || (i_) ==  FCMPWGE || \
                      (i_) == VDCMPWEQ || (i_) == VDCMPWNE || \
                      (i_) == VDCMPWLT || (i_) == VDCMPWLE || \
                      (i_) == VDCMPWGT || (i_) == VDCMPWGE || \
                      (i_) == VFCMPWEQ || (i_) == VFCMPWNE || \
                      (i_) == VFCMPWLT || (i_) == VFCMPWLE || \
                      (i_) == VFCMPWGT || (i_) == VFCMPWGE || \
                      (i_) == VICMPWGT)

                      /*(i_) == FCMPDWNLT || (i_) == FCMPDWNLE || \
                      (i_) == FCMPDWNGT || (i_) == FCMPDWNGE || \
                      (i_) == FCMPWNLT || (i_) == FCMPWNLE || \
                      (i_) == FCMPWNGT || (i_) == FCMPWNGE || \
                      (i_) == VDCMPWNLT || (i_) == VDCMPWNLE || \
                      (i_) == VDCMPWNGT || (i_) == VDCMPWNGE || \
                      (i_) == VFCMPWNLT || (i_) == VFCMPWNLE || \
                      (i_) == VFCMPWNGT || (i_) == VFCMPWNGE)*/

#define IS_CMP(i_) ((i_) == CMP || (i_) == CMPAND || (i_) == CMPS || \
                    (i_) == FCMP || (i_) == FCMPD || \
                    IS_CMPW(i_))
/*      
                    (i_) == VFCMP || \
                    (i_) == VDCMP || \
                    (i_) == CFTBFI || (i_) == CFTBDI || (i_) == FCMPWD)
*/
                     
#define IS_IOPCC(i_) ((i_) == ANDCC || (i_) == SUBCC || (i_) == ANDCC)
/*
 * NOTE: blend operation is different with respect to mem use
 */
#define IS_NOMEM(i_) ((i_) == MUL || (i_) == UMUL || (i_) == DIV || \
                      (i_) == UDIV || (i_) == FCMOV1 || (i_) == FCMOV2 || \
                      (i_) == FCMOVD1 || (i_) == FCMOVD2 || (i_) == VFCMOV1 || \
                      (i_) == VFCMOV2 || (i_) == VDCMOV1 || (i_) == VDCMOV2 || \
                      (i_) == CMOV1 || (i_) == CMOV2 || (i_) == VSCMOV1 || \
                      (i_) == VSCMOV2 || (i_) == VICMOV1 || (i_) == VICMOV2 || \
                      (i_) == LEA2 || (i_) == LEA4 || (i_) == LEA8 )
/*
 * NOTE: FMA4 in AMD can be re ordered, src2 can be mem. need to consider
 * this again while implemting other FMAC like: FMA3 ... 
 */
#define IS_REORD(i_) ((i_) == AND || (i_) == ANDCC || (i_) == XOR || \
                      (i_) == FMUL || (i_) == FADD || \
                      (i_) == FMULD || (i_) == FADDD || \
                      (i_) == VDADD || (i_) == VDMUL || \
                      (i_) == VFADD || (i_) == VFMUL || IS_MAC(i_) )
/*
 * Majedul: These are the instructions where destination is inherently used as
 * one of the sources. This is due to map 4 operands instruction into our 
 * three operand LIL instruction.
 * NOTE: 1. SHUFFLE and ireg2vreg instructions may keep the destination unchanged
 * at certain positions. So, destination is also implicitly used for them
 *       2. DIV, UDIV, DIVS, UDIVS : handled specially 
 */
#define IS_DEST_INUSE_IMPLICITLY(i_) ((i_) == FMAC || (i_) == FMACD || \
                                      (i_) == VFMAC || (i_) == VDMAC || \
                                      (i_) == FCMOV1 || (i_) == FCMOV2 || \
                                      (i_) == FCMOVD1 || (i_) == FCMOVD2 || \
                                      (i_) == VFCMOV1 || (i_) == VFCMOV2 || \
                                      (i_) == VDCMOV1 || (i_) == VDCMOV2 || \
                                      (i_) == CMOV1 || (i_) == CMOV2 || \
                                      (i_) == VSCMOV1 || (i_) == VSCMOV2 || \
                                      (i_) == VICMOV1 || (i_) == VICMOV2 || \
                                      IS_SHUFFLE_OP(i_) || IS_IREG2VREG_OP(i_) )
                                   /*|| IS_MOVS(i_) ) */

#define IS_SELECT_OP(i_) ((i_) == CMOV1   || (i_) == CMOV2  || \
                          (i_) == FCMOV1  || (i_) == FCMOV2 || \
                          (i_) == FCMOVD1 || (i_) == FCMOVD2 )

/*
 * Majedul: 32 bit integer instruction (in X64), may need some special treat
 * since all int registers don't support 32 bit version (like: r8-r15 in X64)
 */
#define IS_SHORT_INT_OP(i_) ( (i_) == LDS    || (i_) == STS || \
                              (i_) == ORS    || (i_) == XORS || \
                              (i_) == NOTS   || (i_) == SHLS || \
                              (i_) == SHLCCS || (i_) == SHRS || \
                              (i_) == SHRCCS || (i_) == SARS || \
                              (i_) == ADDS   || (i_) == ADDCCS || \
                              (i_) == SUBS   || (i_) == SUBCCS || \
                              (i_) == MULS   || (i_) == UMULS || \
                              (i_) == DIVS   || (i_) == UDIVS || \
                              (i_) == CMPS   || (i_) == MOVS || \
                              (i_) == NEGS  )

#define IS_SHUFFLE_OP(i_)   ( (i_) == VFSHUF   || (i_) == VDSHUF || \
                              (i_) == VSSHUF   || (i_) == VISHUF )

#define IS_IREG2VREG_OP(i_) ( (i_) == VGR2VR16 || (i_) == VGR2VR32 || \
                              (i_) == VGR2VR64 )

/*#define IS_MOVS(i_) ( (i_) == VDMOVS || (i_) == VFMOVS )*/

#define IS_PREF(i_) ( (i_) == PREFR || (i_) == PREFW )


INSTQ *NewInst(BBLOCK *myblk, INSTQ *prev, INSTQ *next, enum inst ins,
               short dest, short src1, short src2);
INSTQ *InsNewInst(BBLOCK *myblk, INSTQ *prev, INSTQ *next, enum inst ins,
                  short dest, short src1, short src2);
INSTQ *InsNewInstAfterLabel(BBLOCK *blk, enum inst ins,
                            short dest, short src1, short src2);
void InsInstInBlockList(BLIST *blist, int FIRST, enum inst ins,
                        short dest, short src1, short src2);
INSTQ *DelInst(INSTQ *del);
INSTQ *RemoveInstFromQ(INSTQ *del);
INSTQ *KillThisInst(INSTQ *kp);
void KillAllInst(INSTQ *base);
char *op2str(short op);
void WriteLILToBinFile(char *fname, BBLOCK *bbase);
void ReadLILFromBinFile(char *fname);
void PrintInst(FILE *fpout, BBLOCK *bbase);
void PrintThisInst(FILE *fpout, int i, INSTQ *ip);
void PrintThisInstQ(FILE *fpout, INSTQ *ip);
INSTQ *InsertInstBeforeQEntry(INSTQ *list, INSTQ *add);
int FindInstNum(BBLOCK *blk, INSTQ *inst);
INSTQ *FindFirstLILforHIL(INSTQ *ipX);
#endif
