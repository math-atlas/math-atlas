%{
   #include "ifko.h"
   #include "fko_h2l.h"
   int WhereAt=0, lnno=1;
%}
%union
{
   int inum;
   short sh;
   float  fnum;
   double dnum;
   char str[256];
   char c;
}

%token ROUT_NAME ROUT_LOCALS ROUT_BEGIN ROUT_END RETURN
%token DOUBLE FLOAT INT UINT DOUBLE_PTR FLOAT_PTR INT_PTR UINT_PTR 
%token PARAMS LST ABST
%token <inum> ICONST
%token <fnum> FCONST
%token <dnum> DCONST
%token <str> NAME
%token <c> aop
%right '=' PE
%left OR
%left AND
%left EQ NE
%left '<' '>' LE GE
%left LSHIFT RSHIFT
%left '+' '-'
%left '*' '/' '%'
%right UNMIN NOT

%type <inum> icexpr
%type <fnum> fcexpr
%type <dnum> dcexpr
%type <sh> ID avar iconst fpconst fconst dconst ptrderef

%%

%{
/*
 * Duplicate this for fcexpr
 */
%}
lines : lines line ';' | line ';' ;
line : stateflag | paradec | typedec | statement ;

        /* need to add GLOBALS section */
stateflag: ROUT_NAME NAME 
         {
            if (WhereAt != 0)
               yyerror("Improper ROUTINE statement");
            WhereAt = 1;
            strcpy(rout_name, $2);
fprintf(stderr, "grammer setting rout_name='%s'\n", rout_name);
            STdef(rout_name, T_FUNC | GLOB_BIT, 0);
         }
         | ROUT_LOCALS
         {
            if (WhereAt != 1)
               yyerror("Improper ROUT_LOCALS statement");
            WhereAt = 2;
         }
         | ROUT_BEGIN
         {
            if (WhereAt > 2)
               yyerror("Improper ROUT_BEGIN statement");
            CreateSysLocals();
            NumberLocalsByType();
            CreateLocalDerefs();
            WhereAt = 3;
         }
         | ROUT_END
         {
            if (WhereAt > 3)
               yyerror("Improper ROUT_END statement");
            WhereAt = 4;
         }
	 ;
typedec : INT LST idlist              { declare_list(T_INT); }
        | UINT LST idlist              { declare_list(T_INT | UNSIGNED_BIT); }
        | FLOAT LST idlist           { declare_list(T_FLOAT); }
        | DOUBLE LST idlist           { declare_list(T_DOUBLE); }
        | UINT_PTR LST idlist { declare_list(T_INT | PTR_BIT | UNSIGNED_BIT); }
        | INT_PTR LST idlist          { declare_list(T_INT    | PTR_BIT); }
        | FLOAT_PTR LST idlist        { declare_list(T_FLOAT  | PTR_BIT); }
        | DOUBLE_PTR LST idlist      { declare_list(T_DOUBLE | PTR_BIT); }
	;
paradec : PARAMS LST idlist          { para_list(); } 
        ;
idlist  : idlist ',' NAME            {NewID($3);}
        | NAME                       {NewID($1);}
	;

statement : arith
          | ptrderef '=' ID       {DoArrayStore($1, $3);}
          | ID '=' ptrderef       {DoArrayLoad($1, $3);}
          | ID '=' ID             {DoMove($1, $3);}
	  | ID '=' fpconst	  {DoMove($1, $3);}
          | RETURN avar           {DoReturn($2);}
          | NAME ':'              {DoLabel($1);}
	  ;

icexpr  : icexpr '+' icexpr             {$$ = $1 + $3;}
        | icexpr '-' icexpr             {$$ = $1 - $3;}
        | icexpr '*' icexpr             {$$ = $1 * $3;}
        | icexpr '/' icexpr             
        {
           if ($3 == 0) yyerror("divide by zero");
           else $$ = $1 / $3;
        }
        | icexpr '%' icexpr
        {
           if ($3 == 0) yyerror("modulo by zero");
           else $$ = $1 % $3;
        }
        | NOT icexpr                      {$$ = ($2 == 0) ? 0 : 1;} 
        | '-' icexpr %prec UNMIN        {$$ = -$2;}
        | '(' icexpr ')' { $$ = $2; }
        | ICONST                        {$$ = $1;}
	;
fcexpr  : fcexpr '+' fcexpr             {$$ = $1 + $3;}
        | fcexpr '-' fcexpr             {$$ = $1 - $3;}
        | fcexpr '*' fcexpr             {$$ = $1 * $3;}
        | fcexpr '/' fcexpr             
        {
           if ($3 == 0.0) yyerror("divide by zero");
           else $$ = $1 / $3;
        }
        | '-' fcexpr %prec UNMIN        {$$ = -$2;}
        | '(' fcexpr ')' { $$ = $2; }
        | FCONST                        {$$ = $1;}
	;
dcexpr  : dcexpr '+' dcexpr             {$$ = $1 + $3;}
        | dcexpr '-' dcexpr             {$$ = $1 - $3;}
        | dcexpr '*' dcexpr             {$$ = $1 * $3;}
        | dcexpr '/' dcexpr             
        {
           if ($3 == 0.0) yyerror("divide by zero");
           else $$ = $1 / $3;
        }
        | '-' dcexpr %prec UNMIN        {$$ = -$2;}
        | '(' dcexpr ')' { $$ = $2; }
        | DCONST                        {$$ = $1;}
	;

fpconst : fconst	{$$ = $1; }
        | dconst	{$$ = $1; }
	;
fconst : fcexpr         {$$ = STfconstlookup($1);} ;
dconst : dcexpr         {$$ = STdconstlookup($1);} ;
iconst : icexpr         {$$ = STiconstlookup($1);} ;
ID : NAME               
   {if (!($$ = STstrlookup($1))) fko_error(__LINE__,"unknown ID '%s'", $1); }
   ;
ptrderef : ID '[' iconst ']' { $$ = AddDerefEntry($1, 0, 0, $3); }
         | ID '[' ID ']'    { $$ = AddDerefEntry($1, $3, 0, 0); }
         | ID '[' ID '+' iconst ']' { $$ = AddDerefEntry($1, $3, 0, $5); }
         | ID '[' ID '-' icexpr ']' 
           { $$ = AddDerefEntry($1, $3, 0, STiconstlookup(-$5)); }
         ;
avar : ID               {$$ = $1;}
     | fconst           {$$ = $1;}
     | dconst           {$$ = $1;}
     | iconst           {$$ = $1;}
     ;
arith : ID '=' ID '+' avar {DoArith($1, $3, '+', $5); }
      | ID '=' ID '*' avar {DoArith($1, $3, '*', $5); }
      | ID '=' ID '-' avar {DoArith($1, $3, '-', $5); }
      | ID '=' ID '/' avar {DoArith($1, $3, '/', $5); }
      | ID '=' ID '%' avar {DoArith($1, $3, '%', $5); }
      | ID '=' ID RSHIFT avar {DoArith($1, $3, '>', $5); }
      | ID '=' ID LSHIFT avar {DoArith($1, $3, '<', $5); }
      | ID PE avar         {DoArith($1, $1, '+', $3); }
      | ID '=' ABST        {DoArith($1, $1, 'a', 0); }
      | ID '=' '-' avar    {DoArith($1, $4, 'n', 0); }
      | ID PE ID '*' avar  {DoArith($1, $3, 'm', $5); }
      ;
%%

static struct idlist *idhead=NULL, *paras=NULL;

struct idlist *NewID(char *name)
{
   struct idlist *id;

   assert(name);
   id = malloc(sizeof(struct idlist));
   assert(id);
   id->name = malloc(strlen(name)+1);
   assert(id->name);
   strcpy(id->name, name);
   id->next = idhead;
   idhead = id;
   return(id);
}

void KillIDs(void)
{
   struct idlist *next;
   while(idhead)
   {
      if (idhead->name) free(idhead->name);
      next = idhead->next;
      free(idhead);
      idhead = idhead->next;
   }
}

#if 0
struct idlist *ReverseList(struct idlist *base0)
{
   struct idlist *base=NULL, *next;
   while (base0)
   {
      next = base0->next;
      base0->next = base;
      base = base0;
      base0 = next;
   }
   return(base);
}
#endif

void para_list()
{
   short i=0, n, k;
   struct idlist *id;
   extern int NPARA;

   for (id=idhead, n=0; id; id=id->next, n++);
   while(idhead)
   {
      STdef(idhead->name, 0, n-i);
      idhead = idhead->next;
      i++;
   }
   NPARA = i;
   KillIDs();
}

void declare_list(int flag)
{
   unsigned short si;
   struct idlist *id;
   int i, n;

   if (WhereAt == 1)
   {
      for (id=idhead, n=0; id; id=id->next, n++);
      while(idhead)
      {
         si = STstrlookup(idhead->name);
         if (!si) yyerror("Undeclared parameter");
         STsetflag(si, PARA_BIT | flag);
         idhead = idhead->next;
      }
   }
   else
   {
      while(idhead)
      {
         STdef(idhead->name, flag, 0);
         idhead = idhead->next;
      }
   }
   KillIDs();
}

yyerror(char *msg)
{
   fprintf(stderr, "Line %d: %s\n", lnno, msg);
}
