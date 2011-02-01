;
;	ATL_dJIK30x30x30TN30x30x0_a1.mcr
;
;	ATLAS "Speed of Light" DGEMM() kernel for AMD Athlon
;	Code author: Julian Ruhe (ruheejih@linux.zrz.tu-berlin.de | Julian.Ruhe@t-online.de)
;


%define NB 30

%define ELM1 -15*8
%define ELM2 -14*8
%define ELM3 -13*8
%define ELM4 -12*8
%define ELM5 -11*8
%define ELM6 -10*8
%define ELM7 -9*8
%define ELM8 -8*8
%define ELM9 -7*8
%define ELM10 -6*8
%define ELM11 -5*8
%define ELM12 -4*8
%define ELM13 -3*8
%define ELM14 -2*8
%define ELM15 -1*8
%define ELM16 -0*8
%define ELM17 1*8
%define ELM18 2*8
%define ELM19 3*8
%define ELM20 4*8
%define ELM21 5*8
%define ELM22 6*8
%define ELM23 7*8
%define ELM24 8*8
%define ELM25 9*8
%define ELM26 10*8
%define ELM27 11*8
%define ELM28 12*8
%define ELM29 13*8
%define ELM30 14*8

%define DOTP1 ebp
%define DOTP2 4*edi
%define DOTP3 esi
%define DOTP4 2*edi
%define DOTP5 edi
%define DOTP6 0

%macro OPERATION 2
	%if ELM%1 == 0
	rep
	%endif
	fld qword [eax+DOTP1+ELM%1]		
	fmul st0,st1
	faddp st7

	%if ELM%1 == 0
	rep
	%endif
	fld qword [eax+DOTP2+ELM%1]
	fmul st0,st1
	faddp st6

	%if ELM%1 == 0
	rep
	%endif	
	fld qword [eax+DOTP3+ELM%1]
	fmul st0,st1
	faddp st5

	%if ELM%1 == 0
	rep
	%endif	
	fld qword [eax+DOTP4+ELM%1]
	fmul st0,st1
	faddp st4

	%if ELM%1 == 0
	rep
	%endif
	fld qword [eax+DOTP5+ELM%1]
	fmul st0,st1
	faddp st3

	%if ELM%1 == 0
	rep
	%endif	
	fmul qword [eax+DOTP6+ELM%1]
	faddp st1
	%if ELM%2 == 0
	rep
	%endif
	fld qword [ebx+ELM%2]


	
%endmacro
