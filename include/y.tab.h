
typedef union
#ifdef __cplusplus
	YYSTYPE
#endif

{
   int inum;
   short sh;
   float  fnum;
   double dnum;
   char str[256];
   char c;
} YYSTYPE;
extern YYSTYPE yylval;
# define ROUT_NAME 257
# define ROUT_LOCALS 258
# define ROUT_BEGIN 259
# define ROUT_END 260
# define RETURN 261
# define DOUBLE 262
# define FLOAT 263
# define INT 264
# define UINT 265
# define DOUBLE_PTR 266
# define FLOAT_PTR 267
# define INT_PTR 268
# define UINT_PTR 269
# define PARAMS 270
# define LST 271
# define ABST 272
# define ICONST 273
# define FCONST 274
# define DCONST 275
# define NAME 276
# define aop 277
# define PE 278
# define OR 279
# define AND 280
# define EQ 281
# define NE 282
# define LE 283
# define GE 284
# define LSHIFT 285
# define RSHIFT 286
# define UNMIN 287
# define NOT 288
