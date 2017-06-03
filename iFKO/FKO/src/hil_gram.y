/* #define RCW_DEBUG */
%{
   #ifdef RCW_DEBUG
      #define YYDEBUG 1
   #endif
   #include "fko.h"
   #include "fko_h2l.h"
   #include "fko_arch.h"
   int WhereAt=0, lnno=1;
   int VecIntr=0;
   static short *LMA[8] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
/*
 * Majedul: To store the LOOP MU (with no param)
 * nLMU is the number of those markup
 */
   /*static int nLMU = 2; */
   static short LMU[2] = {0,0};
/*
 * For markups which have both list and an int, we keep 2 separate array of ptr
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
   static short maxunroll=0, itermul=0;
   extern short STderef;
   extern RTMARKUP rtmu;
   
   int yylex(void);
   void yyerror(char *msg);

   struct idlist *NewID(char *name);
   static void UpdateLoop(struct loopq *lp);
   static void Update_RoutMU();
   void HandleListMU(int which);
   void HandleIntMU(int which, int ival);
   void HandleListIntMU(int which, int ival);
   void HandleMU(int which);
   void HandleLoopListMU(int which);
   void HandleLoopIntMU(int which, int ival);
   void HandleLoopListIntMU(int which, int ival);
   void HandleLoopMU(int which);
   void para_list();
   void declare_list(int flag);
   void declare_vector(int flag, int vlen);
   void ConstInit(short id, short con);
   void AddVElem2List(short id);
   void AddDim2List(short id);
   void AddUnroll2List(short id);
   void declare_array(int flag, short dim, char *name);
   short HandleArrayAccess(short id, short dim);
   void HandleUnrollFactor(short ptr, int ndim);
   void HandleArith(short dest, short src0, char op, short src1);
   void HandleMove(short dest, short src);
   void HandleVecInit(short id);
   void HandleVecReduce(short sid, short vid, char op, short ic);
   void HandleVecBroadcast(short vid, short ptrderef);
   void HandlePrefetch(short lvl, short ptrderef, int wpf);
   short HandleVecElem(short vid, int elem);
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


%token ROUT_NAME ROUT_LOCALS ROUT_MARKUP ROUT_BEGIN ROUT_END CONST_INIT RETURN
%token DOUBLE FLOAT INT UINT DOUBLE_PTR FLOAT_PTR INT_PTR UINT_PTR 
%token VDOUBLE VFLOAT
%token VHADD VHMAX VHMIN VHSEL VBROADCAST
%token PREFETCHR PREFETCHW
%token DOUBLE_ARRAY FLOAT_ARRAY INT_ARRAY UINT_ARRAY UNROLL_ARRAY
%token PARAMS LST ABST ME LOOP_BEGIN LOOP_BODY LOOP_END MAX_UNROLL IF GOTO
%token <sh> LIST_MU INT_MU MU LIST_INT_MU
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
/*%type <sh> VecElem*/

%%

lines : lines line 
      | line
      ;
lines : lines line ';' | line ';' ;
line : stateflag
     | paradec ';' 
     | typedec ';'
     | constinit ';'
     | markups
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
            STderef = STdef("_NONLOCDEREF", PTR_BIT|DEREF_BIT, 0);
         }
         | ROUT_LOCALS
         {
            if (WhereAt != 1)
               yyerror("Improper ROUT_LOCALS statement");
            WhereAt = 2;
         }
         | ROUT_MARKUP
         {
            if (WhereAt > 2)
               yyerror("Improper ROUT_MARKUP statement");
            WhereAt = 3;   
         }
         | ROUT_BEGIN
         {
            if (WhereAt > 3)
               yyerror("Improper ROUT_BEGIN statement");
            Update_RoutMU(); 
            CreateLocalDerefs();
            CreateArrColPtrs(); /* can't be called before CreateLocalDerefs() */
            WhereAt = 4;
         }
         | ROUT_END
         {
            if (WhereAt > 4)
               yyerror("Improper ROUT_END statement");
            WhereAt = 5;
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
/*
 * NOTE: LOOP in FKO is a DO-WHILE loop, not FOR loop. 
 */
loop_beg : LOOP_BEGIN ID '=' avar loopsm ',' avar loopsm ',' avar loopsm2
         { $$ = DoLoop($2, $4, $7, $10, $5, $8, $11); VecIntr = 0;}
         | LOOP_BEGIN ID '=' avar loopsm ',' avar loopsm
         { $$ = DoLoop($2, $4, $7, STiconstlookup(1), $5, $8, 2); VecIntr = 0; }
         ;
/*loop_markup : LIST_MU LST idlist { HandleLoopListMU($1); }
            | INT_MU LST icexpr  { HandleLoopIntMU($1, $3); }
            | LIST_INT_MU '(' icexpr ')' LST idlist { HandleLoopListIntMU($1,
                                                           $3); }
            | LIST_INT_MU '(' icexpr ')' LST '*' {HandleLoopListIntMU($1, 
                                                       $3); }
            | MU { HandleLoopMU($1); }
            ;
loop_markups : loop_markups loop_markup ';'
             |
             ;*/
markup : LIST_MU LST idlist { HandleListMU($1); }
            | INT_MU LST icexpr  { HandleIntMU($1, $3); }
            | LIST_INT_MU '(' icexpr ')' LST idlist { HandleListIntMU($1,
                                                           $3); }
            | LIST_INT_MU '(' icexpr ')' LST '*' {HandleListIntMU($1, 
                                                       $3); }
            | MU { HandleMU($1); }
            ;
markups : markups markup ';'
             |
             ;

loop_body : LOOP_BODY statements LOOP_END { DoComment("Done LOOP_BODY"); }
          ;
/*loop : loop_beg loop_markups loop_body    { UpdateLoop($1); }
     ;*/
loop : loop_beg markups loop_body    { UpdateLoop($1); }
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
        | VDOUBLE '(' iconst ')' LST idlist {declare_vector(T_VDOUBLE, $3);}
        | VFLOAT '(' iconst ')' LST idlist {declare_vector(T_VFLOAT, $3);}
        ;

arraydim : '[' ID ']' arraydim {$$=$4+1; AddDim2List($2);} 
         | '[' iconst ']' arraydim { $$=$4+1; AddDim2List($2);} 
         | '[' '*' ']' { $$=1; }
         | '[' ']' { $$=1; }
         ;

/*arraydim : arraydim '[' ID ']' {$$=$1+1; AddDim2List($3); } 
         | arraydim '[' iconst ']' { $$=$1+1; AddDim2List($3); } 
         | '[' '*' ']' { $$=1; }
         | '[' ']' { $$=1; }
         ;*/

/*arrayid : NAME  {fprintf(stderr, "arrayid->NAME\n");}
        ;*/

unrollarraylist : unrollarray ',' unrollarraylist 
        | unrollarray
        ;
unrollarray : ID '(' unrollfactor ')' {HandleUnrollFactor($1, $3);} 
            ;
/*unrollfactor : iconst ',' unrollfactor {$$=$3+1; AddUnroll2List($1);}
             | iconst   { $$=1; AddUnroll2List($1);}
             | '*'      { $$=1; AddUnroll2List(STiconstlookup(1));}
             ;*/
unrollfactor : unrollfactor ',' iconst {$$=$1+1; AddUnroll2List($3);}
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
          | vectorspecial ';'     {VecIntr = 1;}
          | prefetch ';'      
          ;
prefetch :  PREFETCHR '(' iconst  ',' ptrderef ')' {HandlePrefetch($3, $5, 0);} 
         |  PREFETCHW '(' iconst  ',' ptrderef ')' {HandlePrefetch($3, $5, 1);} 
         ;
vectorspecial :  ID '=' initvector {HandleVecInit($1);}
          | ID '=' VBROADCAST '(' ptrderef ')' {HandleVecBroadcast($1, $5);}
          | vectorreduce
	  ;

initvector : '{' vectorelem '}'  
           ;
vectorelem : vectorelem ',' ID       {AddVElem2List($3);}
           | vectorelem ',' fpconst  {AddVElem2List($3);}
           | ID                      {AddVElem2List($1);}
           | fpconst                 {AddVElem2List($1);}
           ;
vectorreduce : ID '=' VHADD '(' ID ')'  {HandleVecReduce($1, $5,'A', 0);}
          | ID '=' VHMAX '(' ID ')'  {HandleVecReduce($1, $5,'M', 0);}
          | ID '=' VHMIN '(' ID ')'  {HandleVecReduce($1, $5,'N', 0);}
          | ID '=' VHSEL '(' ID ',' iconst ')' {HandleVecReduce($1, $5,'S', $7);}

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
/*array_access : '[' ID ']' array_access {$$=$4+1; AddDim2List($2);} 
         | '[' iconst ']' array_access { $$=$4+1; AddDim2List($2);} 
         | '[' ID ']' { $$=1; AddDim2List($2);}
         | '[' iconst ']' { $$=1; AddDim2List($2);}
         ;*/
array_access : array_access '[' ID ']' {$$=$1+1; AddDim2List($3);} 
         | array_access '[' iconst ']' { $$=$1+1; AddDim2List($3);} 
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
     /*| VecElem          {$$ = $1;}*/
     ;
/*VecElem : ID '[' icexpr ']' {$$ = HandleVecElem($1, $3);}*/
        
        /* need to change to if (ID op avar) goto LABEL */
        /* > < NE LE GE * +  / RSHIFT LSHIFT | & ^ */
IFOP : '>'  {$$ = '>';}
     | '<'  {$$ = '<';}
     | NE   {$$ = '!';}
     | EQ   {$$ = '=';}
     | LE   {$$ = 'l';}
     | GE   {$$ = 'g';}
     | '&'    {$$ = '&';}
     | '^'    {$$ = '^';}
     ;
   
ifstate : IF '(' ID IFOP avar ')' GOTO NAME         {DoIf($4, $3, $5, $8);} 
        ;
arith : ID '=' ID '+' avar {HandleArith($1, $3, '+', $5); }
      | ID '=' ID '*' avar {HandleArith($1, $3, '*', $5); }
      | ID '=' ID '-' avar {HandleArith($1, $3, '-', $5); }
      | ID '=' ID '/' avar {HandleArith($1, $3, '/', $5); }
      | ID '=' ID '%' avar {HandleArith($1, $3, '%', $5); }
      | ID '=' ID RSHIFT avar {HandleArith($1, $3, '>', $5); }
      | ID '=' ID LSHIFT avar {HandleArith($1, $3, '<', $5); }
      | ID PE avar         {HandleArith($1, $1, '+', $3); }
      | ID '=' ABST avar   {HandleArith($1, $4, 'a', 0); }
      | ID '=' '-' ID      {HandleArith($1, $4, 'n', 0); }
      | ID PE ID '*' avar  {HandleArith($1, $3, 'm', $5); }
      ;
%%

static struct idlist *idhead=NULL;
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
void HandleArith(short dest, short src0, char op, short src1)
{
/*
 * check whether vector intrinsic is applied!
 */ 
   if (IS_VEC(STflag[dest-1]) || IS_VEC(STflag[src0-1]) || 
       (src1 && IS_VEC(STflag[src1-1])))
       VecIntr = 1;

   DoArith(dest, src0, op, src1); 
}

void HandleMove(short dest, short src)
/*
 * this func acts as a wrapper to call DoMove(). It provides an extra checking
 * for Array 
 */
{
   if (IS_ARRAY(STflag[dest-1]) || IS_ARRAY(STflag[src-1]))
      fko_error(__LINE__, "Array assignment not yet supoorted in HIL!");
   else
   {
/*
 *    check for vector move before calling DoMove
 */
      if (IS_VEC(STflag[dest-1]) || IS_VEC(STflag[src-1])) 
         VecIntr = 1;

      DoMove(dest,src);
   }
}

short HandleArrayAccess(short ptr, short dim)
{
   int val, ur;
   short rt = 0, hdm, ldm, arrid;
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
   extern int VECT_FLAG;
   /*int i, n;*/
   /*short *fptrs, *maptrs;*/
   /*short *spa, *spb;*/
/*
 * check for vector intrinsic
 * If intrinsic HIL code is used, we set the flag VECT_INTRINSIC
 * and update the vflag with appropriate type
 */
   if (VecIntr)
   {
      /*fprintf(stderr, "VecIntr used!\n");*/
      fko_warn(__LINE__, "Vector intrinsic used in HIL code!");
      VECT_FLAG = VECT_FLAG | VECT_INTRINSIC;
      VecIntr = 0;
   }

/*
 * Handle markup with no param  
 * ========================
 *    0. NO_CLEANUP
 *    1. UNSAFE_RC
 */
   if (LMU[0])
      lp->LMU_flag = lp->LMU_flag | LMU_NO_CLEANUP;
   if (LMU[1])
      lp->LMU_flag = lp->LMU_flag | LMU_UNSAFE_RC;
   LMU[0] = LMU[1] = 0;
/*
 * Handle markup with ival param    
 */
   lp->maxunroll = maxunroll;
   lp->itermul  = itermul;
   maxunroll = itermul = 0;
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
#if 0
   for (i=1, n=lp->nopf[0]; i <=n ; i++)
      fprintf(stderr, "%s \n", STname[lp->nopf[i]-1]);
#endif
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
   FinishLoop(lp);
}

void HandleLoopMU(int which)
/*
 * Handles loop markup that doesn't have any parameter. The markups are 
 * encoded by which:
 * 
 * 0 : no cleanup 
 * 1 : unsafe rc due to exceptions 
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

void HandleMU(int which)
{
   if (WhereAt == 3)
      fko_error(__LINE__, "Markup not implemented yet for rout\n");
   else if (WhereAt > 3)
      HandleLoopMU(which);
}

void HandleLoopIntMU(int which, int ival)
/*
 * Handles loop markup involving integer scalar values.  The markups are
 * encoded by which:
 *
 * 0 : Max_unroll - maximum unrolling to try
 * --1 : Write_dep_dist - loop unroll at which a loop-carried write dependence
 *                      will be discovered (0 means there are none) -- deleted
 * 1 : iter_mult - indicates loop iteration is multiple of a number.. 0 means
                   undetermined
 */
{
   switch(which)
   {
   case 0:
      maxunroll = ival;
      break;
   case 1:
      itermul = ival;
      break;
   default:
      fko_error(__LINE__, "Unknown which=%d, file %s", which, __FILE__);
   }
}

void HandleIntMU(int which, int ival)
{
   if (WhereAt == 3)
      fko_error(__LINE__, "Markup not implemented yet for rout\n");
   else if (WhereAt > 3)
      HandleLoopIntMU(which, ival);
}

void HandleLoopListMU(int which)
/*
 * Handles loop markup consisting of simple lists.  The markups are encoded
 * by which:
 *   0: Live_scalars_in
 *   1: Live_scalars_out
 *   2: Dead_arrays_in
 *   3: Dead_arrays_in
 *   4: No_prefetch  ..... only no_prefetch is active! 
 */
{
   int i, n;
   short *sp;
   /*short *cp;*/
   struct idlist *id;
   for (id=idhead,n=0; id; id=id->next) n++;
/*
 * FIXME: need to figure out a way to support '*' for all moving ptr, just like
 * alignment markup. 
 */
   if (n)
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

void HandleListMU(int which)
{
   if (WhereAt == 3)
      fko_error(__LINE__, "Markup not implemented yet for rout\n");
   else if (WhereAt > 3)
      HandleLoopListMU(which); 
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

void HandleListIntMU(int which, int ival)
{
   /*if (WhereAt == 3)
      fprintf(stderr, "inside Rout_markup \n");
   else if (WhereAt > 3)*/
      HandleLoopListIntMU(which, ival);
}

void Update_RoutMU()
{
/*
 *    Handle markup with no param, not yet
 */
   LMU[0] = LMU[1] = 0;
/*
 *    Handle markup with list
 */
   LMA[0] = LMA[1] = LMA[2] = LMA[3] = LMA[4] = NULL;
/*
 *    Handle markup which has a list ids and a value; 
 */
   rtmu.aaligned = LMAA[0];
   rtmu.abalign = LMAB[0];
   rtmu.maaligned = LMAA[1];
   rtmu.mbalign = LMAB[1];
   rtmu.faalign = LMAA[2];
   rtmu.fbalign = LMAB[2];
   
   LMAA[0] = LMAA[1] = LMAA[2] = NULL;
   LMAB[0] = LMAB[1] = LMAB[2] = NULL;
}

void para_list()
{
   short i=0, n;
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
   int n;
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

void declare_vector(int flag, int vlen)
{
   unsigned short si;
   struct idlist *id;
   int n, vl;
/*
 * FIXME: if a variable is in param list but not declare in typedec, there is
 * no way to ctach this in existing implementation, will get seg fault in 
 * later stage (like: create prologue )
 */
/*
 * NOTE: right now, we only support vector code where veclen is directly match 
 * with the vlen of the system. We will relax this restriction later!!! 
 */
   vlen = SToff[vlen-1].i;
   vl = vtype2elem(flag);
   if (vlen != vl )
      fko_error(__LINE__, "VLEN not match with the system : %d, %d!!\n", 
                vlen, vl);

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

void AddVElem2List(short id)
/*
 * taking ST index this function add lda into list
 */
{
/*
 * add extra checking, if needed
 */ 
   AddST2List(id);
}

void HandleVecInit(short vid)
{
   int n;
   int stp, vtp, tchk;
   struct slist *sl;
/*
 * checking for the type of vector (to be initialized)
 */
   if (IS_VDOUBLE(STflag[vid-1]))
   {
      vtp = T_VDOUBLE;
      stp = T_DOUBLE;
   }
   else if (IS_VFLOAT(STflag[vid-1]))
   {
      vtp = T_VFLOAT;
      stp = T_FLOAT;
   }
   else yyerror("unknown vector type");
/*
 * check the type of scalar which must be similar with vector type
 */
   tchk = 0;
   for (sl=shead,n=0; sl; sl=sl->next)
   {
      tchk += (FLAG2TYPE(STflag[sl->id-1]) & stp) ? 0: 1 ;
#if 0
      fprintf(stderr, "%s\n", STname[sl->id-1]);
#endif
      n++;
   }
   if (tchk)
      yyerror("type mismatch!!");
      
   if (n != vtype2elem(vtp))
      yyerror("elem count mismatch with vector type");
/*
 * add LIL for this statement
 */

   DoVecInit(vid, shead); 

#if 0
   fprintf(stderr, "%s : %d\n", STname[vid-1], vtp);
   //exit(0);
#endif
   KillSlist();
}

void HandleVecReduce(short sid, short vid, char op, short ic)
{
   int stp=0, vtp, pos;
/*
 * checking for the type of vector and scalar
 */
   if (IS_VDOUBLE(STflag[vid-1]))
   {
      vtp = T_VDOUBLE;
      stp = T_DOUBLE;
   }
   else if (IS_VFLOAT(STflag[vid-1]))
   {
      vtp = T_VFLOAT;
      stp = T_FLOAT;
   }
   else yyerror("unknown vector type");
   
   if (!(stp & FLAG2TYPE(STflag[sid-1])))
      yyerror("type mismatch!");
   
   if (ic) 
      pos = SToff[ic-1].i;
   else pos = 0;

   if (op == 'S' )
   {
      if (pos < 0 || pos > (vtype2elem(vtp)-1) )
         yyerror("incorrect position param for VHSEL operation!");
   }

   DoReduce(sid, vid, op, ic);
}

void HandleVecBroadcast(short vid, short ptrderef)
{
   short vtype, ptype;
   ptype = FLAG2TYPE(STflag[SToff[ptrderef-1].sa[0]-1]);
   vtype = FLAG2TYPE(STflag[vid-1]);
   
   if ( (IS_VFLOAT(vtype) && !IS_FLOAT(ptype)) 
       || (IS_VDOUBLE(vtype) && !IS_DOUBLE(ptype)) )
       //yyerror("type mismatch!");
       fko_error(__LINE__,"type mismatch[%d,%d] !", vtype, ptype);
   
   DoArrayBroadcast(vid, ptrderef);
}

#if 0
short HandleVecElem(short vid, int elem)
{
   short st;
   st = FindSTVecElem(vid, elem+1);
   assert(st);
   return(st);
}
#endif

void HandlePrefetch(short lvl, short ptrderef, int wpf)
{
/*
 * call h2l function to insert instructions
 */

   DoArrayPrefetch(lvl, ptrderef, wpf);
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

void yyerror(char *msg)
{
   fprintf(stderr, "\n\nERROR: Line %d: %s\n\n", lnno, msg);
   exit(-1);
}
