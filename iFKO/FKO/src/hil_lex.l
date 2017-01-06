%{
#include "y.tab.h"
   extern int lnno;
%}
ICON	[0-9]+
HCON	"0x"[0-9,A-F]+
ID	[A-Za-z][A-Za-z0-9_]*
ROUTNAM "ROUTINE"[ \t\v]+ID
%%
"//"[^\n]*\n    { lnno++; strcpy(yylval.str, yytext+2); return(TCOMMENT); }
[\t \v]		{}
[\n]    {lnno++;}
[-]?{ICON}"."{ICON}([eE][-+]?{ICON})?[fF] { sscanf(yytext, "%f", &yylval.fnum);  
       return(FCONST); }
[-]?{ICON}"\."{ICON}([eE][-+]?{ICON})? { yylval.dnum = atof(yytext); 
       return(DCONST); }
{ICON}	{ yylval.inum = atoi(yytext); return(ICONST); }
{HCON}    { sscanf(yytext, "%x", &yylval.inum);  return(ICONST); }
"-" |
"(" | 
")" | 
"!" |
"+" | 
"*" | 
"/" |
"%" | 
"<" | 
">" | 
"&" | 
"," |
";" | 
"[" |
"]" |
":" |
"{" |
"}" |
"="	{ return(yytext[0]); }
"-="    { return(ME); }
"+="    { return(PE); }
"||"    { return(TOR); }
"&&"    { return(TAND); }
"=="    { return(EQ); }
"!="    { return(NE); }
"<="    { return(LE); }
">="    { return(GE); }
"<<"    { return(LSHIFT); }
">>"    { return(RSHIFT); }
"::"    { return(LST); }
"LOOP_BODY" { return(LOOP_BODY); }
"LOOP_END" { return(LOOP_END); }
"LIVE_SCALARS_IN"  { yylval.sh = 0; return(LIST_MU); }
"LIVE_SCALARS_OUT" { yylval.sh = 1; return(LIST_MU); }
"DEAD_ARRAYS_IN"   { yylval.sh = 2; return(LIST_MU); }
"DEAD_ARRAYS_OUT"  { yylval.sh = 3; return(LIST_MU); }
"NO_PREFETCH"      { yylval.sh = 4; return(LIST_MU); }
"MAX_UNROLL"       { yylval.sh = 0; return(INT_MU); }
"ITER_MULT"        { yylval.sh = 1; return(INT_MU); }
"ALIGNED"          { yylval.sh = 0; return(LIST_INT_MU); }
"MUTUALLY_ALIGNED" { yylval.sh = 1; return(LIST_INT_MU); }
"FORCE_ALIGN"      { yylval.sh = 2; return(LIST_INT_MU); }
"NO_CLEANUP"   { yylval.sh = 0; return(MU); }
"UNSAFE_RC"   { yylval.sh = 1; return(MU); }
"LOOP"  { return(LOOP_BEGIN); }
"DOUBLE" { return(DOUBLE); }
"FLOAT" { return(FLOAT); }
"INT" { return(INT); }
"UINT" { return(UINT); }
"DOUBLE_PTR" { return(DOUBLE_PTR); }
"FLOAT_PTR" { return(FLOAT_PTR); }
"INT_PTR" { return(INT_PTR); }
"UINT_PTR" { return(UINT_PTR); }
"DOUBLE_ARRAY" { return(DOUBLE_ARRAY); }
"FLOAT_ARRAY" { return(FLOAT_ARRAY); }
"INT_ARRAY" { return(INT_ARRAY); }
"UINT_ARRAY" { return(UINT_ARRAY); }
"ROUTINE" { return(ROUT_NAME); }
"UNROLL_ARRAY" { return(UNROLL_ARRAY); }
"ROUT_LOCALS" { return(ROUT_LOCALS); }
"ROUT_MARKUP" { return(ROUT_MARKUP); }
"CONST_INIT" { return(CONST_INIT); }
"ROUT_BEGIN" { return(ROUT_BEGIN); }
"RETURN"   { return(RETURN); }
"ROUT_END" { return(ROUT_END); }
"PARAMS"   { return(PARAMS); }
"ABS"      { return(ABST); }
"IF"       { return(IF); }
"GOTO"     { return(GOTO); }
"VDOUBLE"    { return(VDOUBLE); }
"VFLOAT"    { return(VFLOAT); }
"_VHADD"    { return(VHADD); }
"_VHMAX"    { return(VHMAX); }
"_VHMIN"    { return(VHMIN); }
"_VHSEL"    { return(VHSEL); }
"_VBROADCAST"    { return(VBROADCAST); }
"_PREFETCHR"    { return(PREFETCHR); }
"_PREFETCHW"    { return(PREFETCHW); }
{ID}      { sprintf(yylval.str, "%s", yytext); return(NAME); }

%%
