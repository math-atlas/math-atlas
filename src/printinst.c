#include "fko.h"
#include <stdlib.h>

int main(int nargs, char **args)
{
   int what=(-1);
   enum inst i;
   char *inststr;

   if (nargs > 1)
      what = atoi(args[1]);

   for (i=0; i != LAST_INST; i++)
   {
      if (what != -1) i = what;
      switch(i)
      {
      case UNIMP:
         inststr = "UNIMP";
	 break;
      case NOP:
         inststr = "NOP";
	 break;
      case LABEL:
         inststr = "LABEL";
	 break;
      case CMPFLAG:
         inststr = "CMPFLAG";
	 break;
      case LD:
         inststr = "LD";
	 break;
      case ST:
         inststr = "ST";
	 break;
      case SHL:
         inststr = "SHL";
	 break;
      case SHR:
         inststr = "SHR";
	 break;
      case SAR:
         inststr = "SAR";
	 break;
      case ADD:
         inststr = "ADD";
	 break;
      case SUB:
         inststr = "SUB";
	 break;
      case MUL:
         inststr = "MUL";
	 break;
      case UMUL:
         inststr = "UMUL";
	 break;
      case DIV:
         inststr = "DIV";
	 break;
      case UDIV:
         inststr = "UDIV";
	 break;
      case CMP:
         inststr = "CMP";
	 break;
      case MOV:
         inststr = "MOV";
	 break;
      case NEG:
         inststr = "NEG";
	 break;
      case LDS:
         inststr = "LDS";
	 break;
      case STS:
         inststr = "STS";
	 break;
      case SHLS:
         inststr = "SHLS";
	 break;
      case SHRS:
         inststr = "SHRS";
	 break;
      case SARS:
         inststr = "SARS";
	 break;
      case ADDS:
         inststr = "ADDS";
	 break;
      case SUBS:
         inststr = "SUBS";
	 break;
      case CMPS:
         inststr = "CMPS";
	 break;
      case MOVS:
         inststr = "MOVS";
	 break;
      case NEGS:
         inststr = "NEGS";
	 break;
      case JMP:
         inststr = "JMP";
	 break;
      case JEQ:
         inststr = "JEQ";
	 break;
      case JNE:
         inststr = "JNE";
	 break;
      case JLT:
         inststr = "JLT";
	 break;
      case JLE:
         inststr = "JLE";
	 break;
      case JGT:
         inststr = "JGT";
	 break;
      case JGE:
         inststr = "JGE";
	 break;
      case PREFR:
         inststr = "PREFR";
	 break;
      case PREFW:
         inststr = "PREFW";
	 break;
      case RET:
         inststr = "RET";
	 break;
      case PREFRS:
         inststr = "PREFRS";
	 break;
      case PREFWS:
         inststr = "PREFWS";
	 break;
      case FLD:
         inststr = "FLD";
	 break;
      case FST:
         inststr = "FST";
	 break;
      case FMAC:
         inststr = "FMAC";
	 break;
      case FMUL:
         inststr = "FMUL";
	 break;
      case FDIV:
         inststr = "FDIV";
	 break;
      case FADD:
         inststr = "FADD";
	 break;
      case FSUB:
         inststr = "FSUB";
	 break;
      case FABS:
         inststr = "FABS";
	 break;
      case FCMP:
         inststr = "FCMP";
	 break;
      case FNEG:
         inststr = "FNEG";
	 break;
      case FMOV:
         inststr = "FMOV";
	 break;
      case FLDD:
         inststr = "FLDD";
	 break;
      case FSTD:
         inststr = "FSTD";
	 break;
      case FMACD:
         inststr = "FMACD";
	 break;
      case FMULD:
         inststr = "FMULD";
	 break;
      case FDIVD:
         inststr = "FDIVD";
	 break;
      case FADDD:
         inststr = "FADDD";
	 break;
      case FSUBD:
         inststr = "FSUBD";
	 break;
      case FABSD:
         inststr = "FABSD";
	 break;
      case FCMPD:
         inststr = "FCMPD";
	 break;
      case FNEGD:
         inststr = "FNEGD";
	 break;
      case FMOVD:
         inststr = "FMOVD";
	 break;
      case VFLD:
         inststr = "VFLD";
	 break;
      case VFST:
         inststr = "VFST";
	 break;
      case VFABS:
         inststr = "VFABS";
	 break;
      case VFMAC:
         inststr = "VFMAC";
	 break;
      case VFMUL:
         inststr = "VFMUL";
	 break;
      case VFDIV:
         inststr = "VFDIV";
	 break;
      case VFADD:
         inststr = "VFADD";
	 break;
      case VFSUB:
         inststr = "VFSUB";
	 break;
      case VFCMP:
         inststr = "VFCMP";
	 break;
      case VSHUF:
         inststr = "VSHUF";
	 break;
      case CVTIS:
         inststr = "CVTIS";
	 break;
      case CVTSI:
         inststr = "CVTSI";
	 break;
      case CVTFI:
         inststr = "CVTFI";
	 break;
      case CVTIF:
         inststr = "CVTIF";
	 break;
      case CVTDI:
         inststr = "CVTDI";
	 break;
      case CVTID:
         inststr = "CVTID";
	 break;
      case CVTFS:
         inststr = "CVTFS";
	 break;
      case CVTSF:
         inststr = "CVTSF";
	 break;
      case CVTDS:
         inststr = "CVTDS";
	 break;
      case CVTSD:
         inststr = "CVTSD";
	 break;
      case LAST_INST:
         inststr = "UNIMP";
	 break;
      default:
         inststr = "UNKNOWN";
      }
      printf("Instruction %d = '%s'\n", i, inststr);
      if (what != -1) break;
   }
   return(0);
}
