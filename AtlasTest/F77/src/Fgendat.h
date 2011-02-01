#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

#ifdef __STDC__
#include <stdarg.h>
#define STDC_ARGS(p)           p
#else
#include <varargs.h>
#define STDC_ARGS(p)           ()
#endif
/*
 * This isn't perfect, but it should avoid missing symbols for Windows installs
 */
#ifdef ATL_OS_WinNT
   #define lrand48 rand
   #define srand48 srand
#endif


#define MAXK          16
#define MAXN2        50
#define MAXN3        20

#define MAXAB          7
#define MAXNK          7
#define MAXNN          9

#define MAXFILES      20

#define MAX( x, y ) ( (x) > (y) ? (x) : (y) )
#define MIN( x, y ) ( (x) < (y) ? (x) : (y) )
