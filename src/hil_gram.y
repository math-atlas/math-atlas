/* #define RCW_DEBUG */
%{
   #ifdef RCW_DEBUG
      #define YYDEBUG 1
   #endif
   #include "fko.h"
   #include "fko_h2l.h"
   #include "fko_arch.h"
   int WhereAt=0, lnno=1;
   static short *LMA[8] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
/*
 * Majedul: To store the LOOP MU (with no param)
 * Right now, there are two: 0. MUTUALLY_UNALIGNED 1. .... 
 * Need to generalize all markups in well design. 
 * nLMU is the number of those markup
 */
   static int nLMU = 2; 
   static short LMU[2] = {0,0};
/*
 * For markup which has both list and an int, we keep 2 separate array of ptr
 * NOTE: following array of ptr stores ptr with element count at position 0,
 * unlike LMA
 */
   static short *LMAA[3] = {NULL,NULL,NULL};
   static short *LMAB[3] = {NULL,NULL,NULL};
/*   
   static short *aalign=NULL;
   static short *balign=NULL;
   static short *faalign=NULL;
   static short *fbalign=NULL;
   static short *maalign=NULL;
   static short *mbalign=NULL;
*/
   static short maxunroll=0, writedd=1;
   extern short STderef;

   struct idlist *NewID(char *name);
   static void UpdateLoop(struct loopq *lp);
   void HandleLoopListMU(int which);
   void HandleLoopIntMU(int which, int ival);
   void HandleLoopListIntMU(int which, int ival);
   void HandleLoopMU(int which);
   void para_list();
   void declare_list(int flag);
   void ConstInit(short id, short con);
   void AddDim2List(short id);
   void AddUnroll2List(short id);
   void declare_array(int flag, short dim, char *name);
   short HandleArrayAccess(short id, short dim);
   void HandleUnrollFactor(short ptr, int ndim);
   void HandleMove(short dest, short src);
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
%token DOUBLE_ARRAY FLOAT_ARRAY INT_ARRAY UINT_ARRAY UNROLL_ARRAY
%token PARAMS LST ABST ME LOOP_BEGIN LOOP_BODY LOOP_END MAX_UNROLL IF GOTO
%token <sh> LOOP_LIST_MU LOOP_INT_MU LOOP_MU LOOP_LIST_INT_MU
%token <inum> ICONST
%token <fnum> FCONST
%token <dnum> DCONST
%token <str> NAME
%token <str> TCOMMENT
%token <c> aop
%right '=' PE
%left TOR
%left TAND
%left EQ NE
%left '<' '>' LE GE
%left LSHIFT RSHIFT
%left '+' '-'
%left '*' '/' '%'
%right UNMIN TNOT
%left LSMU

%type <inum> icexpr
%type <fnum> fcexpr
%type <dnum> dcexpr
%type <sh> ID avar const iconst fpconst fconst dconst ptrderef 
%type <sh> loopsm loopsm2
%type <lq> loop_beg
%type <c> IFOP
%type <inum> arraydim
%type <inum> array_access
%type <inum> unrollfactor

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
            #ifdef RCW_DEBUG
               yydebug = 1;
            #endif
            if (WhereAt != 0)
               yyerror("Improper ROUTINE statement");
            WhereAt = 1;
            strcpy(rout_name, $2);
            STdef(rout_name, T_FUNC | GLOB_BIT, 0);
            STderef = STderef = STdef("_NONLOCDEREF", PTR_BIT|DEREF_BIT, 0);
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
            
            CreateLocalDerefs();
            CreateArrColPtrs(); /* can't be called before CreateLocalDerefs() */
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
         | '{' '*' '}'     { $$ = 3; }
         |                 { $$ = 2; }
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
            | LOOP_INT_MU LST icexpr  { HandleLoopIntMU($1, $3); }
            | LOOP_LIST_INT_MU '(' icexpr ')' LST idlist { HandleLoopListIntMU($1,
                                                           $3); }
            | LOOP_LIST_INT_MU '(' icexpr ')' LST '*' {HandleLoopListIntMU($1, 
                                                       $3); }
 /*            | MAX_UNROLL   LST icexpr { maxunroll = $3; } */
            | LOOP_MU { HandleLoopMU($1); }
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
	| DOUBLE_ARRAY arraydim LST NAME {declare_array(T_DOUBLE|PTR_BIT|ARRAY_BIT
        ,$2,$4); }
	| FLOAT_ARRAY arraydim LST NAME {declare_array(T_FLOAT|PTR_BIT|ARRAY_BIT
        ,$2,$4); }
        | UNROLL_ARRAY LST unrollarraylist {}
        ;

/*        | DOUBLE_ARRAY LST arraylist {fprintf(stderr, "DOUBLE_ARRAY :: arraylist\n");}
	| FLOAT_ARRAY LST arraylist {fprintf(stderr, "FLOAT_ARRAY :: arraylist\n");}
        ;*/
/*NOTE: declaration for array, will handle later */
/*arraylist : arraylist ',' arraydec {fprintf(stderr, "arraylist->arraylist , array\n");}
          | arraydec               {fprintf(stderr, "arraylist -> array\n");}
          ;*/
/*arraydec : ID arraydim { fprintf(stderr, "array -> ID arraydim\n");}
      ;*/

arraydim : '[' ID ']' arraydim {$$=$4+1; AddDim2List($2);} 
         | '[' iconst ']' arraydim { $$=$4+1; AddDim2List($2);} 
         | '[' '*' ']' { $$=1; }
         | '[' ']' { $$=1; }
         ;
/*arrayid : NAME  {fprintf(stderr, "arrayid->NAME\n");}
        ;*/

unrollarraylist : unrollarray ',' unrollarraylist 
        | unrollarray
        ;
unrollarray : ID '(' unrollfactor ')' {HandleUnrollFactor($1, $3);} 
            ;
unrollfactor : iconst ',' unrollfactor {$$=$3+1; AddUnroll2List($1);}
             | iconst   { $$=1; AddUnroll2List($1);}
             | '*'      { $$=1; AddUnroll2List(STiconstlookup(1));}
             ;
paradec : PARAMS LST idlist          { para_list(); } 
        ;
constinit : CONST_INIT LST initlist
        ;
idlist  : idlist ',' NAME            {NewID($3);}
        | NAME                       {NewID($1);}
	;
/*initlist : inititem ',' inititem
         | inititem
         ;*/
initlist : initlist ',' inititem
         | inititem
         ;
inititem :  ID '=' iconst { ConstInit($1, $3); }
         |  ID '=' fconst { ConstInit($1, $3); }
         |  ID '=' dconst { ConstInit($1, $3); }
         ;

comment : TCOMMENT { DoComment($1); }
        ;
statements : statements statement
           | statement
           ;
statement : arith ';'
          | comment
          | ptrderef '=' ID ';'   {DoArrayStore($1, $3);}
          | ID '=' ptrderef ';'   {DoArrayLoad($1, $3);}
          /*| ID '=' ID       ';'   {DoMove($1, $3);}
	  | ID '=' const ';'	  {DoMove($1, $3);}*/
          | ID '=' ID ';'         {HandleMove($1, $3);}
          | ID '=' const ';'      {HandleMove($1, $3);}
          | RETURN avar  ';'      {DoReturn($2);}
          | RETURN ';'            {DoEmptyReturn();}
          | NAME ':'              {DoLabel($1);}
          | GOTO NAME ';'         {DoGoto($2);}
          | ifstate ';'
          /*| loop*/ /* added so that loop can be nested */
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
        | TNOT icexpr                      {$$ = ($2 == 0) ? 0 : 1;} 
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
/*ptrderef : ID '[' icexpr ']' { $$ = AddArrayDeref($1, 0, $3); }
         | ID '[' ID ']'    { $$ = AddArrayDeref($1, $3, 0); }
         | ID '[' ID '+' icexpr ']' { $$ = AddArrayDeref($1, $3, $5); }
         | ID '[' ID '-' icexpr ']' { $$ = AddArrayDeref($1, $3, -$5); }
         ;
*/
ptrderef : ID array_access { $$ = HandleArrayAccess($1, $2);}
         | ID '[' ID '+' icexpr ']' { $$ = AddArrayDeref($1, $3, $5); }
         | ID '[' ID '-' icexpr ']' { $$ = AddArrayDeref($1, $3, -$5); }
         ;
array_access : '[' ID ']' array_access {$$=$4+1; AddDim2List($2);} 
         | '[' iconst ']' array_access { $$=$4+1; AddDim2List($2);} 
         | '[' ID ']' { $$=1; AddDim2List($2);}
         | '[' iconst ']' { $$=1; AddDim2List($2);}
         ;

fconst : fcexpr         {$$ = STfconstlookup($1);} ;
dconst : dcexpr         {$$ = STdconstlookup($1);} ;
iconst : icexpr         {$$ = STiconstlookup($1);} ;
ID : NAME               
   {
   if (!($$ = STstrlookup($1))) fko_error(__LINE__,"unknown ID '%s'", $1); }
   ;
avar : ID               {$$ = $1;}
     | fconst           {$$ = $1;}
     | dconst           {$$ = $1;}
     | iconst           {$$ = $1;}
     ;
                /* need to change to if (ID op avar) goto LABEL */
        /* > < NE LE GE * +  / RSHIFT LSHIFT | & ^ */
IFOP : '>'  {$$ = '>';}
     | '<'  {$$ = '<';}
     | NE   {$$ = '!';}
     | LE   {$$ = 'l';}
     | GE   {$$ = 'g';}
     | '&'    {$$ = '&';}
     | '^'    {$$ = '^';}
     ;
   
ifstate : IF '(' ID IFOP avar ')' GOTO NAME         {DoIf($4, $3, $5, $8);} 
        ;
arith : ID '=' ID '+' avar {DoArith($1, $3, '+', $5); }
      | ID '=' ID '*' avar {DoArith($1, $3, '*', $5); }
      | ID '=' ID '-' avar {DoArith($1, $3, '-', $5); }
      | ID '=' ID '/' avar {DoArith($1, $3, '/', $5); }
      | ID '=' ID '%' avar {DoArith($1, $3, '%', $5); }
      | ID '=' ID RSHIFT avar {DoArith($1, $3, '>', $5); }
      | ID '=' ID LSHIFT avar {DoArith($1, $3, '<', $5); }
      | ID PE avar         {DoArith($1, $1, '+', $3); }
      | ID '=' ABST avar   {DoArith($1, $4, 'a', 0); }
      | ID '=' '-' ID      {DoArith($1, $4, 'n', 0); }
      | ID PE ID '*' avar  {DoArith($1, $3, 'm', $5); }
      ;
%%

static struct idlist *idhead=NULL, *paras=NULL;
static struct slist *shead=NULL;

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

struct slist *AddST2List(short stid)
{
   struct slist *new;
   new = malloc(sizeof(struct slist));
   assert(new);
   new->id = stid;
   new->next = shead;
   shead = new;
   return(new);

}

void KillSlist(void)
{
   struct slist *next;
   while(shead)
   {
      next = shead->next;
      free(shead);
      shead = next;
   }
}

void AddDim2List(short id)
/*
 * taking ST index this function add lda into list
 */
{
   int flag;
   flag = STflag[id-1];

   if (IS_LOCAL(flag) || IS_CONST(flag) || IS_PARA(flag))
   {
      if (!IS_INT(FLAG2TYPE(STflag[id-1])))
         fko_error(__LINE__, "ARRAY dimension var/const must be integer\n");
   }
   else
      fko_error(__LINE__, "UNKNOWN ARRAY DIMENSION\n");

   AddST2List(id);
}

void AddUnroll2List(short id)
{
   if (!IS_CONST(STflag[id-1]) || !IS_INT(FLAG2TYPE(STflag[id-1])))
   {
      fko_error(__LINE__, "UNROLL FACTOR MUST BE INTEGER\n");
   }
   AddST2List(id);
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

void HandleMove(short dest, short src)
/*
 * this func acts as a wrapper to call DoMove(). It provides an extra checking
 * for Array 
 */
{
   if (IS_ARRAY(STflag[dest-1]) || IS_ARRAY(STflag[src-1]))
      fko_error(__LINE__, "Array assignment not yet supoorted in HIL!");
   else
      DoMove(dest,src);
}

short HandleArrayAccess(short ptr, short dim)
{
   int i, val, ur;
   struct slist *sl;
   short rt, hdm, ldm, arrid;
   short ptr1d;
   extern int FKO_FLAG; /* global flag for opt array access */
/*
 * access array with 1D indexing, like: A[2]
 */
   if (dim == 1) /* execute old code to access the 1D array */
   {
/*
 *    we don't support pointer assignment for 2D array yet, but can be 
 *    supported later, like:  
 *    DOUBLE_ARRAY :: A[lda][*];
 *    DOUBLE_PTR :: Pa;
 *    pa = A[1]; ==> pa = _A1_S ; 
 */
/*
 *    NOTE: 1D array doesn't have entry in STarr table and sa[3] of ST entry
 *    should be zero(0) .
 *    FIXME: we will eventually support pointer assignment for 2D array, like:
 *             a = A[1]; 
 *           if A is a 2D pointer, then it will copy the column pointer, like:
 *             a = A1;
 */
      if (SToff[ptr-1].sa[3] && STarr[SToff[ptr-1].sa[3]-1].ndim != dim)
         fko_error(__LINE__,"Access of array %s doesn't match declaration\n", 
                 STname[ptr-1]);         
/*
 *    Handle 1D array just like the old way 
 */
      if (IS_CONST(STflag[shead->id-1]))
      {
         /*ptrderef : ID '[' icexpr ']' { $$ = AddArrayDeref($1, 0, $3); }*/
         val = SToff[shead->id-1].i;
         rt = AddArrayDeref(ptr, 0, val);     
      }
      else
      {
         /*| ID '[' ID ']'    { $$ = AddArrayDeref($1, $3, 0); } */
         rt = AddArrayDeref(ptr, shead->id, 0);
      }
   }
/*
 * here comes the 2D array access. higher dimension will be translated as 
 * a pointer (created and stored in STarr) so that we can call same 
 * AddArrayDeref function
 */
   else if (dim == 2) /* access fo 2D array */
   {
      hdm = shead->id;        /* higher dimension 1st */
      ldm = shead->next->id;  /* lower diumension */
/*
 *    should match the dimension of array access with its declaration
 */
      if (STarr[SToff[ptr-1].sa[3]-1].ndim != dim)
         fko_error(__LINE__,"Access of 2D array %s doesn't match declaration\n", 
                 STname[ptr-1]);        
/*
 *    we only support constant indexing now, will extend to support index 
 *    variable later
 */
      if (IS_CONST(STflag[hdm-1]) && IS_CONST(STflag[ldm-1]))
      {
         arrid = SToff[ptr-1].sa[3];
         if (!STarr[arrid-1].urlist)
            fko_error(__LINE__, "Unroll factor not defined!!!");
         ur = STarr[arrid-1].urlist[0];
         ur = SToff[ur-1].i;
         if ( SToff[hdm-1].i < ur)
         {
         #ifdef X86         
            if (FKO_FLAG & IFF_OPT2DPTR) /* applying optimization */
            {
               rt = AddOpt2dArrayDeref(arrid, hdm, ldm, ur);   
            }
            else
            {
               ptr1d = STarr[arrid-1].colptrs[SToff[hdm-1].i+1];
#if 0      
               fprintf(stderr, "%s [%d][%d] ==> ", STname[ptr-1], SToff[hdm-1].i, 
                       SToff[ldm-1].i);
               fprintf(stderr, "%s [%d]\n", STname[ptr1d-1], SToff[ldm-1].i);
#endif
               rt = AddArrayDeref(ptr1d, 0, SToff[ldm-1].i);      
            }
         #else
            ptr1d = STarr[arrid-1].colptrs[SToff[hdm-1].i+1];
            rt = AddArrayDeref(ptr1d, 0, SToff[ldm-1].i);      
         #endif
         }
         else
            fko_error(__LINE__, 
                     "Unroll factor is smaller than the index accessed");

      }
      else
         fko_error(__LINE__, "only const index is supported for 2D array");
   }
   else
      fko_error(__LINE__, "NOT SUPPORTED DIMENSION MORE THAN 2 FOR ARRAY!\n");

   KillSlist();

   return rt;
}

void HandleUnrollFactor(short ptr, int ndim)
{
   int i;
   short id;
   struct slist *sl;
   short *url;
#if 0
   fprintf(stderr, "%s#%d: ", STname[ptr-1], ndim);
   for (sl=shead; sl; sl=sl->next)
   {
      fprintf(stderr, "%d ", SToff[sl->id-1].i);
   }
   fprintf(stderr, "\n");
#endif
   id = STarrlookup(ptr);
   if (!id) 
      fko_error(__LINE__,"ARRAY %s NOT DEFINED YET\n", STname[ptr-1]); 
   else if (STarr[id-1].ndim != ndim)
      fko_error(__LINE__,"DIMENSION OF ARRAY %s NOT MATCH WITH UNROLL FACTOR\n", 
                STname[ptr-1]); 

   url = malloc(ndim*sizeof(short));
   assert(url);
   for (i=0, sl=shead; sl && i<ndim; sl=sl->next, i++)
   {
         url[i] = sl->id;    
   }
   UpdateSTarrUnroll(id, url );
   free(url);
   KillSlist();
}

static void UpdateLoop(struct loopq *lp)
{
   int i, j, k, n, m;
   /*short *fptrs, *maptrs;*/
   short *spa, *spb;
/*
 * Handle markup with param  
 * ========================
 * Right now, we don't have meaningful markup with empty param
 */
   LMU[0] = LMU[1] = 0;
/*
 * Handle markup with ival param    
 */
   lp->maxunroll = maxunroll;
   lp->writedd  = writedd;
   maxunroll = writedd = 0;
/*
 * Handle markup with list.
 * ========================
 * Following markups are in this category but most of them are not 
 * implemented/active yet:
 *    0. LIVE_SCALARS_IN
 *    1. LIVE_SCALARS_OUT
 *    2. DEAD_ARRAYS_IN
 *    3. DEAD_ARRAYS_OUT
 *    4. NO_PREFETCH
 * only NO_PREFETCH markup is active
 */
#if 0
   lp->vslivein  = LMA[0];
   lp->vsliveout = LMA[1];
   lp->vstmp = LMA[2];
   lp->varrs = LMA[3];
#endif
   lp->nopf     = LMA[4];
   LMA[0] = LMA[1] = LMA[2] = LMA[3] = LMA[4] = NULL;

/*
 * Handle markup which has a list ids and a value; 
 * ==============================================
 * Note: that value is saved as list representing value for each id.
 * LMAA[] : list of ids
 * LMAB[] : list of byte; this value would be same for all ids if the markup
 * is used only once; otherwise, may differ. 
 * LMAA[0] => aaligned
 * LMAA[1] => maaligned
 * LMAA[2] => faalign
 * LMAB[0] => abalign
 * LMAB[1] => mbalign
 * LMAB[2] => fbalign
 */
   lp->aaligned = LMAA[0];
   lp->abalign = LMAB[0];
   lp->maaligned = LMAA[1];
   lp->mbalign = LMAB[1];
   lp->faalign = LMAA[2];
   lp->fbalign = LMAB[2];
   LMAA[0] = LMAA[1] = LMAA[2] = NULL;
   LMAB[0] = LMAB[1] = LMAB[2] = NULL;
#if 0
   //spa = lp->faalign;
   //spb = lp->fbalign;
   //spa = lp->maaligned;
   //spb = lp->mbalign;
   spa = lp->aaligned;
   spb = lp->abalign;
   if (spa)
   {
      fprintf(stderr, "ptrs=");
      for (i=1,n=spa[0]; i<=n; i++)
      {
         fprintf(stderr, " %s[%d]", STname[spa[i]-1], spb[i]);
      }
      fprintf(stderr, "\n");
   }
   else if (spb)
   {
      fprintf(stderr, " *[%d]\n", spb[1]);
   }
#endif

#if 0
/*
 * Majedul: if there is a LOOP MARKUP for alignment, update the loopq 
 * structure. check all the markup one by one. after updating re-init to 0.
 * look into fko_types.h for the macro.
 * alignment should be for a number of byte!!!
 */

   if (LMA[5]) /* force align ptr */
   {
      fptrs = LMA[5];
      if (fptrs[0] != 1)
         fko_error(__LINE__, "More than one ptr to force align!");
      /*fprintf(stderr, "Force Align (%d) = %s\n", 
                fptrs[0], STname[fptrs[1]-1]);*/
      lp->falign = fptrs;
   }
   LMA[5] = NULL;

   if (LMU[1]) /* mutually not aligned!! */
   {
      /*fprintf(stderr, "mutually unaligned!!!\n");*/
      if (lp->malign)
        fko_error(__LINE__, "Inconsistancy in malign declaration!\n"); 
      lp->malign = -1;
   }
/*
 * mutually_aligned has the higest priority... I may skip mutually unaligned 
 * later
 */
   if (LMAU[0])  /* mutually_aligned (byte ) :: id, id */
   {
      /*lp->malign = malign */
      fptrs = LMAU[0];
      if (lp->falign) /* we have a ptr to force */
      {
         for (n=fptrs[0],i=1; i<=n; i++)
         {
            if (lp->falign[1] == fptrs[i])
               break;
         }
         if (i<=n ) /* found force ptr in mutually align ptrs */
         {
/*
 *          add all the mutually aligned ptrs in falign but place force ptr in 
 *          fligned[1] position
 */
            maptrs = malloc(sizeof(short)*(n+1));
            maptrs[0] = n;
            m = 1;
            maptrs[m++] = lp->falign[1];
            for (j=1; j < i; j++) /* add first set of ptrs */
            {
               maptrs[m++] = fptrs[j];
            }
            for (k=i+1; k <=n; k++) /* last set of ptrs */
            {
               maptrs[m++] = fptrs[k];
            }
            free(fptrs);
            lp->falign = maptrs;
         }
      }
      else /* no ptr to force */
      {
         /*lp->falign = fptrs;*/ // the list is reserved 
         n = fptrs[0];
         maptrs = malloc(sizeof(short)*(n+1));
         maptrs[0] = n;
         for (i=n,j=1; i >= 1; i--,j++)
            maptrs[j] = fptrs[i];
         free(fptrs);
         lp->falign = maptrs;
      }
#if 0
      fprintf(stderr, "maptrs=");
      for (i=1,n=lp->falign[0]; i<=n; i++)
         fprintf(stderr, " %s", STname[lp->falign[i]-1]);
      fprintf(stderr, "\n");
#endif
   }
/*
 * If Mutually_aligned (32) :: * is specify, lp->malign is updated with byte 
 * size; though it doesn't change the lp->falign
 */
   /*else*/
   LMAU[0] = NULL;
#endif   
   FinishLoop(lp);
}

void HandleLoopMU(int which)
/*
 * Handles loop markup that doesn't have any parameter. The markups are 
 * encoded by which:
 * 
 * 0 : mutually_unaligned ... that means, all ptr can't be aligned with simple 
       loop peeling ---- not used anymore
 * 1 : ... ... ...   
 */
{
   switch(which)
   {
   case 0: 
      LMU[0] = 1;
      break;
   case 1:
      LMU[1] = 1;
      break;
   default:
      fko_error(__LINE__, "Unknown which=%d, file %s", which, __FILE__);
   }
}

void HandleLoopIntMU(int which, int ival)
/*
 * Handles loop markup involving integer scalar values.  The markups are
 * encoded by which:
 *
 * 0 : Max_unroll - maximum unrolling to try
 * 1 : Write_dep_dist - loop unroll at which a loop-carried write dependence
 *                      will be discovered (0 means there are none)
 * 2 : mutually_aligned_for 
 */
{
   switch(which)
   {
   case 0:
      maxunroll = ival;
      break;
   case 1:
      writedd = ival;
      break;
/*
   case 2:
      malign = ival;
      break;
*/      
   default:
      fko_error(__LINE__, "Unknown which=%d, file %s", which, __FILE__);
   }
}
void HandleLoopListMU(int which)
/*
 * Handles loop markup consisting of simple lists.  The markups are encoded
 * by which:
 *   0: Live_scalars_in
 *   1: Live_scalars_out
 *   2: Dead_arrays_in
 *   3: Dead_arrays_in
 *   4: No_prefetch
 *   following are not used anymore
 *   //5: Force_aligned
 *   //100: Array_aligned
 */
{
   int i, n;
   short *sp;
   short *cp;
   struct idlist *id;
   for (id=idhead,n=0; id; id=id->next) n++;
#if 0
   if (which == 100)
   {
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
#endif   
   {
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
      LMA[which] = sp - 1; /* list of id with count at position 0 */
   }
   KillIDs();
}

void HandleLoopListIntMU(int which, int ival)
/*
 * Handles loop markup consisting of simple lists and a value.  The markups are 
 * encoded by which:
 * 0 : Aligned array list with byte 
 * 1 : Mutually_aligned array list with byte as alignment
 * 2 : force align array list with byte of alignmnet
 */
{
   int i, n, n0;
   struct idlist *id;
   short *spa, *spb;
   for (id=idhead,n=0; id; id=id->next) n++;
/*
 * alignment of all the arrays would be the ival
 */
   if (LMAA[which]) /* already exists */
   {
      n0 = LMAA[which][0]; 
      spa = malloc(sizeof(short)*(n+n0+1));
      spb = malloc(sizeof(short)*(n+n0+1));
      assert(spa && spb);
      spa[0] = spb[0] = n + n0;
      
      for (i=1; i<=n0; i++)
      {
         spa[i] = LMAA[which][i];
         spb[i] = LMAB[which][i];
      }
      for (id=idhead; i <= (n+n0); i++,id=id->next)
      {
         spa[i] = STstrlookup(id->name);
         if (!spa[i])
            fko_error(__LINE__, "Unknown ID: %s which=%d\n", id->name, which);
         spb[i] = ival;
      }
/*
 *    we save the spa and spb without excluding the count at the position 0
 */
      /*LMAU[which] = spa - 1;*/
      free(LMAA[which]);
      free(LMAB[which]);
      LMAA[which] = spa;
      LMAB[which] = spb;
   }
   else /* new list */
   {
      if (n)
      {
         spa = malloc(sizeof(short)*(n+1));
         spb = malloc(sizeof(short)*(n+1));
         assert(spa && spb);
         spa[0] = spb[0] = n;
         for (i=1,id=idhead; i <= n; i++, id=id->next)
         {
            spa[i] = STstrlookup(id->name);
            if (!spa[i])
               fko_error(__LINE__, "Unknown ID: %s which=%d\n", id->name, which);
            spb[i] = ival; 
         }
         LMAA[which] = spa;
         LMAB[which] = spb;
      }
/*
 *    '*' is used in markup, which indicates all ptrs/array; But there is no
 *    way to identify array without analysis here. So, mark the LMAA[] as NULL 
 *    but set LMAB[].  
 */
      else /* using * as list which set empty list */
      {
         if (!LMAB[which]) /* not set yet */
         {
            spb = malloc(sizeof(short)*2);
            spb[0] = 1;
            spb[1] = ival;
            LMAB[which] = spb;
         }
         else /* we have already set one, overwrite it with larger value*/
         {
            assert(LMAB[which][0]==1); 
            LMAB[which][1] = (LMAB[which][1] >= ival)? LMAB[which][1] : ival;
         }
      }
   }
   KillIDs();

#if 0
   if (which == 0)
   {
      if (idhead)
      {
         malign = ival;    
         assert(!LMAU[which]);
         sp = malloc(sizeof(short)*(n+1));
         assert(sp);
         *sp++ = n;
         for (i=0,id=idhead; i != n; i++, id=id->next)
         {
            sp[i] = STstrlookup(id->name);
            if (!sp[i])
               fko_error(__LINE__, "Unknown ID: %s which=%d\n", id->name, which);
         }
         LMAU[which] = sp - 1; /* list of id with count at position 0 */
         KillIDs();
      }
      else /* all ptr are mutually aligned, MUTUALLY_ALIGNED(INT):: * */
      {
         LMAU[which] = NULL;
         malign = ival;
      }
   }
   else
      fko_error(__LINE__, "Unknown markup (which=%d)\n", which);
#endif
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
/*
 * FIXME: if a variable is in param list but not declare in typedec, there is
 * no way to ctach this in existing implementation, will get seg fault in 
 * later stage (like: create prologue )
 */

   if (WhereAt == 1) /* declared as parameter */
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
   else /* declared as local */
   {
      while(idhead)
      {
         STdef(idhead->name, LOCAL_BIT | flag, 0);
         idhead = idhead->next;
      }
   }
   KillIDs();
}

void declare_array(int flag, short dim, char *name)
{
   int i;
   struct slist *sl;
   unsigned short si;
   short *ldas;
/*
 * NOTE: 1D array is always treated as PTR as we avoid array declaration for
 * 1D array right now.
 */
   if (dim==1)
      fko_error(__LINE__, "1D array should be treated as pointer\n");

   if (WhereAt == 1) /* inside parameter declaration */
   {
      si = STstrlookup(name); 
      if (!si) yyerror("Undeclared parameter");
      STsetflag(si, PARA_BIT | flag);
      ldas = malloc((dim-1)*sizeof(short));
      assert(ldas);
      for (i=0, sl=shead; sl && i<(dim-1); sl=sl->next, i++)
      {
         ldas[i] = sl->id;    
      }
      STsetArray(si, dim, ldas);
      free(ldas);
#if 0
      fprintf(stderr,"ARRAY = %s: ",name);
      fprintf(stderr, "dim=%d ,",dim);
      fprintf(stderr, "lda=");
      for (sl=shead; sl; sl=sl->next)
      {
         if (IS_CONST(STflag[sl->id-1]))
            fprintf(stderr, "%d, ", SToff[sl->id-1].i);
         else
            fprintf(stderr, "%s, ", STname[sl->id-1]);
      }
      fprintf(stderr, "\n");
#endif      
      KillSlist();
   }
   else /* inside local declaration */
   { 
      si = STstrlookup(name); 
      if (si) fko_error(__LINE__,"Variable=%s already exists\n", name);
      for (sl=shead; sl; sl=sl->next)
      {
         if (!IS_CONST(STflag[sl->id-1]))
            fko_error(__LINE__, "dimension can't be variable for local array\n");
      }
      KillSlist();
      fko_error(__LINE__, "Local array not supported yet!");
   }
}

void ConstInit(short id, short con)
{
   extern struct locinit *LIhead;
   if (FLAG2TYPE(STflag[id-1]) != FLAG2TYPE(STflag[con-1]))
   {
      fprintf(stderr, "id='%s', idtype=%d, contype=%d\n", STname[id-1],
              FLAG2TYPE(STflag[id-1]), FLAG2TYPE(STflag[con-1]));
      fko_error(__LINE__, "Type mismatch in CONST_INIT\n");
   }
   LIhead = NewLI(id, con, LIhead);
}

yyerror(char *msg)
{
   fprintf(stderr, "\n\nERROR: Line %d: %s\n\n", lnno, msg);
   exit(-1);
}
