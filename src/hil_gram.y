%{
/*   #define YYDEBUG 1  */
   #include "ifko.h"
   #include "fko_h2l.h"
   #include "fko_arch.h"
   int WhereAt=0, lnno=1;
   static short *LMA[8] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
   static short *aalign=NULL;
   static uchar *balign=NULL;
   static short maxunroll=0;
%}
%union
{
   int inum;
   short sh;
   float  fnum;
   double dnum;
   char str[512];
   char c;
   struct loopq *lq;
}


%token ROUT_NAME ROUT_LOCALS ROUT_BEGIN ROUT_END CONST_INIT RETURN
%token DOUBLE FLOAT INT UINT DOUBLE_PTR FLOAT_PTR INT_PTR UINT_PTR 
%token PARAMS LST ABST ME LOOP_BEGIN LOOP_BODY LOOP_END MAX_UNROLL
%token <sh> LOOP_LIST_MU
%token <inum> ICONST
%token <fnum> FCONST
%token <dnum> DCONST
%token <str> NAME
%token <str> COMMENT
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
%left LSMU

%type <inum> icexpr
%type <fnum> fcexpr
%type <dnum> dcexpr
%type <sh> ID avar const iconst fpconst fconst dconst ptrderef 
%type <sh> loopsm loopsm2
%type <lq> loop_beg

%%

lines : lines line 
      | line
      ;
lines : lines line ';' | line ';' ;
line : stateflag
     | paradec ';' 
     | typedec ';'
     | constinit ';'
     | statement
     | loop 
     ;
stateflag: ROUT_NAME NAME 
         {
            #ifdef YYDEBUG
               yydebug = 1;
            #endif
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
            #ifdef X86_64
               CreateLocalDerefs(8);
            #else
               CreateLocalDerefs(4);
            #endif
            WhereAt = 3;
         }
         | ROUT_END
         {
            if (WhereAt > 3)
               yyerror("Improper ROUT_END statement");
            WhereAt = 4;
         }
	 ;
loopsm2  : '{' '+' '}'     { $$ = 2; } 
         | '{' '-' '}'     { $$ = -2; }
         |                 { $$ = 0; }
         ;
loopsm   : '{' PE '}'  { $$ = 1; }
         | '{' ME '}'  { $$ = -1; } 
         | '{' '+' '}' { $$ = 2; } 
         | '{' '-' '}' { $$ = -2; }
         |             { $$ = 0; }
         ;
loop_beg : LOOP_BEGIN ID '=' avar loopsm ',' avar loopsm ',' avar loopsm2
         { $$ = DoLoop($2, $4, $7, $10, $5, $8, $11); }
         | LOOP_BEGIN ID '=' avar loopsm ',' avar loopsm
         { $$ = DoLoop($2, $4, $7, STiconstlookup(1), $5, $8, 2); }
         ;
loop_markup : LOOP_LIST_MU LST idlist { HandleLoopListMU($1); }
            | MAX_UNROLL   LST icexpr { maxunroll = $3; }
            ;
loop_markups : loop_markups loop_markup ';'
             |
             ;
loop_body : LOOP_BODY statements LOOP_END { DoComment("Done LOOP_BODY"); }
          ;
loop : loop_beg loop_markups loop_body    { UpdateLoop($1); }
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
constinit : CONST_INIT LST initlist
        ;
idlist  : idlist ',' NAME            {NewID($3);}
        | NAME                       {NewID($1);}
	;
initlist : inititem ',' inititem
         | inititem
         ;
inititem :  ID '=' iconst { ConstInit($1, $3); }
         |  ID '=' fconst { ConstInit($1, $3); }
         |  ID '=' dconst { ConstInit($1, $3); }
         ;

comment : COMMENT { DoComment($1); }
        ;
statements : statements statement
           | statement
           ;
statement : arith ';'
          | comment
          | ptrderef '=' ID ';'   {DoArrayStore($1, $3);}
          | ID '=' ptrderef ';'   {DoArrayLoad($1, $3);}
          | ID '=' ID       ';'   {DoMove($1, $3);}
	  | ID '=' const ';'	  {DoMove($1, $3);}
          | RETURN avar  ';'      {DoReturn($2);}
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

const   : fpconst	{$$ = $1; }
        | iconst	{$$ = $1; }
        ;
fpconst : fconst	{$$ = $1; }
        | dconst	{$$ = $1; }
	;
ptrderef : ID '[' icexpr ']' { $$ = AddArrayDeref($1, 0, $3); }
         | ID '[' ID ']'    { $$ = AddArrayDeref($1, $3, 0); }
         | ID '[' ID '+' icexpr ']' { $$ = AddArrayDeref($1, $3, $5); }
         | ID '[' ID '-' icexpr ']' { $$ = AddArrayDeref($1, $3, -$5); }
         ;
fconst : fcexpr         {$$ = STfconstlookup($1);} ;
dconst : dcexpr         {$$ = STdconstlookup($1);} ;
iconst : icexpr         {$$ = STiconstlookup($1);} ;
ID : NAME               
   {
   fprintf(stderr, "strlookup %s -> %d\n", $1, STstrlookup($1));
   if (!($$ = STstrlookup($1))) fko_error(__LINE__,"unknown ID '%s'", $1); }
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
      | ID '=' '-' ID      {DoArith($1, $4, 'n', 0); }
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
      idhead = next;
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

static void UpdateLoop(struct loopq *lp)
{
fprintf(stderr, "%s(%d)\n", __FILE__, __LINE__);
/* InsNewInst(NULL, NULL, COMMENT, STstrconstlookup("start UpdateLoop"), 0, 0); */
   lp->maxunroll = maxunroll;
   lp->slivein  = LMA[0];
   lp->sliveout = LMA[1];
   lp->adeadin  = LMA[2];
   lp->adeadout = LMA[3];
   lp->nopf     = LMA[4];
   lp->aaligned = aalign;
   lp->abalign  = balign;
   LMA[0] = LMA[1] = LMA[2] = LMA[3] = LMA[4] = aalign = NULL;
   balign = NULL;
   maxunroll = 0;
   FinishLoop(lp);
fprintf(stderr, "%s(%d)\n", __FILE__, __LINE__);
}

void HandleLoopListMU(int which)
/*
 * Handles loop markup consisting of simple lists.  The markups are incoded
 * by which:
 *   0: Live_scalars_in
 *   1: Live_scalars_out
 *   2: Dead_arrays_in
 *   3: Dead_arrays_in
 *   4: No_prefetch
 * 100: Array_aligned
 */
{
   int i, n;
   short *sp;
   char *cp;
   struct idlist *id;
   for (id=idhead,n=0; id; id=id->next) n++;

fprintf(stderr, "%s(%d), which=%d\n", __FILE__, __LINE__, which);
   if (which == 100)
   {
fprintf(stderr, "%s(%d), which=%d\n", __FILE__, __LINE__, which);
      assert(!aalign && !balign);
      n >>= 1;
      sp = malloc(sizeof(short)*(n+1));
      cp = malloc(sizeof(uchar)*n);
      assert(sp && cp);
      *sp++ = n;
      for (i=0,id=idhead; i != n; i++)
      {
         sp[i] = STstrlookup(id->name);
         if (!sp[i])
            fko_error(__LINE__, "Unknown ID: %s which=%d\n", id->name, which);
         id = id->next;
         cp[i] = atoi(id->name);
         id = id->next;
      }
      aalign = sp - 1;
      balign = cp;
   }
   else
   {
fprintf(stderr, "%s(%d), which=%d\n", __FILE__, __LINE__, which);
      assert(!LMA[which]);
      sp = malloc(sizeof(short)*(n+1));
      assert(sp);
      *sp++ = n;
      for (i=0,id=idhead; i != n; i++, id=id->next)
      {
         sp[i] = STstrlookup(id->name);
         if (!sp[i])
            fko_error(__LINE__, "Unknown ID: %s which=%d\n", id->name, which);
      }
      LMA[which] = sp - 1;
   }
fprintf(stderr, "%s(%d), which=%d\n", __FILE__, __LINE__, which);
   KillIDs();
fprintf(stderr, "%s(%d)\n", __FILE__, __LINE__);
}

void para_list()
{
   short i=0, n, k;
   struct idlist *id;
   extern int NPARA;

   for (id=idhead, n=0; id; id=id->next, n++);
   while(idhead)
   {
      STdef(idhead->name, 0, n-i);
fprintf(stderr, "\n %d. %s\n", n-i, idhead->name);
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
         STdef(idhead->name, LOCAL_BIT | flag, 0);
         idhead = idhead->next;
      }
   }
   KillIDs();
}

struct locinit *NewLI(short id, short con, struct locinit *next)
{
   struct locinit *lp;
   lp = malloc(sizeof(struct locinit));
   assert(lp);
   lp->id = id;
   lp->con = con;
   lp->next = next;
   return(lp);
}

void ConstInit(short id, short con)
{
   extern struct locinit *LIhead;
   if (FLAG2TYPE(STflag[id-1]) != FLAG2TYPE(STflag[con-1]))
      fko_error(__LINE__, "Type mismatch in CONST_INIT\n");
   LIhead = NewLI(id, con, LIhead);
}
yyerror(char *msg)
{
   fprintf(stderr, "Line %d: %s\n", lnno, msg);
}
