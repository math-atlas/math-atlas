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
{ICON}"."{ICON}[e,E]{ICON} { sscanf(yytext, "%f", &yylval.fnum);  return(FCONST); }
{ICON}"\."{ICON}([d,D]ICON)? { yylval.dnum = atof(yytext); 
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
"LIVE_SCALARS_IN"  { yylval.sh = 0; return(LOOP_LIST_MU); }
"LIVE_SCALARS_OUT" { yylval.sh = 1; return(LOOP_LIST_MU); }
"DEAD_ARRAYS_IN"   { yylval.sh = 2; return(LOOP_LIST_MU); }
"DEAD_ARRAYS_OUT"  { yylval.sh = 3; return(LOOP_LIST_MU); }
"NO_PREFETCH"      { yylval.sh = 4; return(LOOP_LIST_MU); }
"MAX_UNROLL"       { yylval.sh = 0; return(LOOP_INT_MU); }
"WRITE_DEP_DIST"   { yylval.sh = 1; return(LOOP_INT_MU); }
"LOOP"  { return(LOOP_BEGIN); }
"DOUBLE" { return(DOUBLE); }
"FLOAT" { return(FLOAT); }
"INT" { return(INT); }
"UINT" { return(UINT); }
"DOUBLE_PTR" { return(DOUBLE_PTR); }
"FLOAT_PTR" { return(FLOAT_PTR); }
"INT_PTR" { return(INT_PTR); }
"UINT_PTR" { return(UINT_PTR); }
"ROUTINE" { return(ROUT_NAME); }
"ROUT_LOCALS" { return(ROUT_LOCALS); }
"CONST_INIT" { return(CONST_INIT); }
"ROUT_BEGIN" { return(ROUT_BEGIN); }
"RETURN"   { return(RETURN); }
"ROUT_END" { return(ROUT_END); }
"PARAMS"   { return(PARAMS); }
"ABS"      { return(ABST); }
"IF"       { return(IF); }
"GOTO"     { return(GOTO); }
{ID}      { sprintf(yylval.str, "%s", yytext); return(NAME); }

%%
