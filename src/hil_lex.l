%{
#include "y.tab.h"
   extern int lnno;
%}
ICON	[-+]?[0-9]+
HCON	"0x"[0-9,A-F]+
FCON	{ICON}"."{ICON}[e,E]ICON
DCON	{ICON}"."{ICON}([d,D]ICON)?
ID	[A-Za-z][A-Za-z0-9_]*
ROUTNAM "ROUTINE"[ \t\v]+ID
%%
[\t \v]		{}
[\n]    {lnno++;}
"-" |
"(" | 
")" | 
"!" |
"*" | 
"/" |
"%" | 
"+" | 
"<" | 
">" | 
"&" | 
"," |
";" | 
"="	{ return(yytext[0]); }
"+="    { return(PE); }
"||"    { return(OR); }
"&&"    { return(AND); }
"=="    { return(EQ); }
"!="    { return(NE); }
"<="    { return(LE); }
">="    { return(GE); }
"<<"    { return(LSHIFT); }
">>"    { return(RSHIFT); }
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
"ROUT_BEGIN" { return(ROUT_BEGIN); }
"RETURN"   { return(RETURN); }
"ROUT_END" { return(ROUT_END); }
"PARAMS"   { return(PARAMS); }
"ABS"      { return(ABST); }
{ID}      { sprintf(yylval.str, "%s", yytext); return(NAME); }
{ICON}	{ yylval.inum = atoi(yytext); return(ICONST); }
{HCON}    { sscanf(yytext, "%x", &yylval.inum);  return(ICONST); }
{FCON}    { sscanf(yytext, "%f", &yylval.fnum);  return(FCONST); }
{DCON}    { yylval.dnum = atof(yytext); 
fprintf(stderr, "returning DCON of %f\n", yylval.dnum);
return(DCONST); }

"::"    { return(LST); }
%%
