#ifndef FKO_INST_H
#define FKO_INST_H
enum comp_flag
/*
 * enumerated type for compiler flag, so format is:
 * CMPFLAG, <enum comp_flag>, <info1>, <info2>
 */
{
   CF_LOOP_INIT,  /* loop# */
   CF_LOOP_BODY,
   CF_LOOP_UPDATE,
   CF_LOOP_TEST,
   CF_LOOP_END
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
 */
enum inst
{
   UNIMP,                       /* NULL, NULL, NULL */
   NOP,                         /* NULL, NULL, NULL */
   COMMENT,                     /* <string>,NULL,NULL */
   LABEL,                       /* <labname>,NULL,NULL */
   CMPFLAG,                     /* flag,flag,flag compiler info */
   LOOP_BEG,                    /* <lnum> */
   LOOP_END,                    /* <lnum> */
/*
 * 32-bit integer
 */
   LD,                          /* [r], [ptr], NULL */
   ST,                          /* [ptr], [r], NULL */
   SHL,                         /* r0 = r1 << r/c */
   SHR,                         /* r0 = r1 >> r/c, set [cc] */
   SAR,                         /* r0 = r1 >> r/c */
   ADD,                         /* r0 = r1 + r/c */
   SUB,                         /* r0 = r1 - r/c */
   MUL,                         /* r0 = r1 * r/c */
   UMUL,                        /* r0 = r1 * r/c, unsigned */
   DIV,                         /* r0 = r1 / r/c */
   UDIV,                      
   CMP,                         /* NULL, r1, r2/c: set [cc] based on r1 - r2 */
   MOV,                         /* [r0], [r1/c] : r0 = r1 */
   NEG,                         /* [r0], [r1] : r0 = -r1 */
/*   ABS, ; abs commented out because not widely supported */
/*
 * 64-bit integer
 */
   LDL,                         /* [r], [ptr], NULL */
   STL,                         /* [ptr], [r], NULL */
   SHLL, SHLCCL,                /* r0 = r1 << r/c */
   SHRL, SHRCCL,                /* r0 = r1 >> r/c, set [cc] */
   SARL,                        /* r0 = r1 >> r/c */
   ADDL, ADDCCL,                /* r0 = r1 + r/c */
   SUBL, SUBCCL,                /* r0 = r1 - r/c */
   MULL,                        /* r0 = r1 * r/c */
   UMULL,                       /* r0 = r1 * r/c, unsigned */
   DIVL,                        /* r0 = r1 / r/c */
   UDIVL,
   CMPL,                        /* set [cc] based on r0 - r1 */
   MOVL,                        /* [r0], [r1] : r0 = r1 */
   NEGL,                        /* [r0], [r1] : r0 = -r1 */
   ABSL,
/*
 * Jump instructions
 */
   JMP,                         /* LABEL */
   JEQ, JNE, JLT, JLE, JGT, JGE,
   PREFR,                       /* [ptr] [ilvl], NULL */
   PREFW,                       /* [ptr] [ilvl], NULL */
   RET,                         /* NULL,NULL,NULL : return to caller */
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
   FLD,                         /* [fr] [ptr] */
   FST,                         /* [ptr], [fr] */
   FMAC,                        /* fr0 += fr1 * fr2 */
   FMUL,                        /* fr0 = fr1 * fr2 */
   FDIV,                        /* fr0 = fr1 / fr2 */
   FADD,                        /* fr0 = fr1 + fr2 */
   FSUB,                        /* fr0 = fr1 - fr2 */
   FABS,                        /* [fr0], [fr1] : fr0 = abs(fr1) */
   FCMP,                        /* [fr0], [fr1] : set [cc] by (fr0 - fr1) */
   FNEG,                        /* [fr0], [fr1] : fr0 = -fr1 */
   FMOV,                        /* fr0 = fr1 */
/*
 * Floating point instructions, double precision
 */
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
   FMOVD,                       /* fr0 = fr1 */
/*
 * Vector instructions, add to these later (double, scalar).
 */
   VFLD,
   VFST,
   VFABS,
   VFMAC,
   VFMUL,
   VFDIV,
   VFADD,
   VFSUB,
   VFCMP,
   VSHUF,
/*
 * Type conversion instructions
 */
   CVTIL,
   CVTLI,
   CVTFI,
   CVTIF,
   CVTDI,
   CVTID,
   CVTFL,
   CVTLF,
   CVTDL,
   CVTLD,
   LAST_INST
};

INSTQ *NewInst(INSTQ *prev, INSTQ *next, enum inst ins,
               short dest, short src1, short src2);
INSTQ *InsNewInst(INSTQ *prev, INSTQ *next, enum inst ins,
                  short dest, short src1, short src2);
INSTQ *DelInst(INSTQ *del);
void KillAllInst(INSTQ *base);

#endif
