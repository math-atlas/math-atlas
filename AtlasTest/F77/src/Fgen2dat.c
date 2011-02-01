#include "Fgendat.h"

main()
{
   FILE           * ofp;
   char           * str = NULL;
   int            i, itest, k, kfile;
   int            kval[MAXNK], nval[MAXNN];
   float          alphaR[MAXAB], alphaI[MAXAB], betaR[MAXAB], betaI[MAXAB];

   str = (char * ) malloc( 13 );

   srand48( (long)( time( NULL ) ) );

   for( kfile = 0; kfile < MAXFILES; kfile++ )
   {
      for( i = 0; i < MAXNN; i++ ) nval[i] = lrand48() % MAXN2;

      for( i = 0; i < MAXNK; i++ ) kval[i] = lrand48() % MAXK;

      for( i = 0; i < MAXAB; i++ )
      {
         alphaR[i] = ( ( lrand48() % 100 ) / 10.0 ) - 5.0;
         alphaI[i] = ( ( lrand48() % 100 ) / 10.0 ) - 5.0;
         betaR [i] = ( ( lrand48() % 100 ) / 10.0 ) - 5.0;
         betaI [i] = ( ( lrand48() % 100 ) / 10.0 ) - 5.0;
      }
/*
*  Single precision real
*/
      strcpy( str, "sblat2_" );
      if( kfile >= 9 )
      { str[7]='0'+(kfile+1)/10; str[8]='0'+(kfile+1)%10; str[9]='\0';
      strcpy( &str[9], ".dat" ); }
      else { str[7] = '1' + kfile; str[8] = '\0'; strcpy( &str[8], ".dat" ); }
      if( !( ofp = fopen( str, "w" ) ) )
      { fprintf( stderr, "Error in opening the file ... quit\n" ); exit( 1 ); }

      fprintf( ofp, "%s\n", "'SBLAT2.SUMM'     NAME OF SUMMARY OUTPUT FILE'"  );
      fprintf( ofp, "%s\n", "6                 UNIT NUMBER OF SUMMARY FILE'"  );
      fprintf( ofp, "%s\n", "'SBLAT2.SNAP'     NAME OF SNAPSHOT OUTPUT FILE'" );
      fprintf( ofp, "%s\n",
      "-1                UNIT NUMBER OF SNAPSHOT FILE (NOT USED IF .LT. 0)" );
      fprintf( ofp, "%s\n",
      "F        LOGICAL FLAG, T TO REWIND SNAPSHOT FILE AFTER EACH RECORD." );
      fprintf( ofp, "%s\n", "F        LOGICAL FLAG, T TO STOP ON FAILURES." );
      fprintf( ofp, "%s\n", "T        LOGICAL FLAG, T TO TEST ERROR EXITS." );
      fprintf( ofp, "%s\n", "16.0     THRESHOLD VALUE OF TEST RATIO"        );

      fprintf( ofp, "%d%s\n", MAXNN, "                 NUMBER OF VALUES OF N" );
      for( i = 0; i < MAXNN; i++ ) fprintf( ofp,  "%d ", nval[ i ] );
      fprintf( ofp, "\t%s\n", "VALUES OF N" );
      fprintf( ofp, "%d%s\n", MAXNK, "                 NUMBER OF VALUES OF K" );
      for( i = 0; i < MAXNK; i++ ) fprintf( ofp,  "%d ", kval[ i ] );
      fprintf( ofp, "\t%s\n", "VALUES OF K" );

      fprintf( ofp, "%s\n",
      "4                 NUMBER OF VALUES OF INCX AND INCY" );
      fprintf( ofp, "%s\n", "1 2 -1 -2         VALUES OF INCX AND INCY" );

      fprintf( ofp, "%d%s\n", MAXAB,
      "                 NUMBER OF VALUES OF ALPHA" );
      for( i = 0; i < MAXAB; i++ ) fprintf( ofp,  "%6.3f ", alphaR[ i ] );
      fprintf( ofp, "\t%s\n", "VALUES OF ALPHA" );
      fprintf( ofp, "%d%s\n", MAXAB,
      "                 NUMBER OF VALUES OF BETA " );
      for( i = 0; i < MAXAB; i++ ) fprintf( ofp,  "%6.3f ", betaR [ i ] );
      fprintf( ofp, "\t%s\n", "VALUES OF BETA " );

      fprintf( ofp, "%s\n", "SGEMV  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "SGBMV  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "SSYMV  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "SSBMV  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "SSPMV  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "STRMV  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "STBMV  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "STPMV  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "STRSV  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "STBSV  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "STPSV  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "SGER   T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "SSYR   T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "SSPR   T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "SSYR2  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "SSPR2  T PUT F FOR NO TEST. SAME COLUMNS." );

      fclose( ofp );
/*
 * Double precision real
 */
      strcpy( str, "dblat2_" );
      if( kfile >= 9 )
      { str[7]='0'+(kfile+1)/10; str[8]='0'+(kfile+1)%10; str[9]='\0';
      strcpy( &str[9], ".dat" ); }
      else { str[7] = '1' + kfile; str[8] = '\0'; strcpy( &str[8], ".dat" ); }
      if( !( ofp = fopen( str, "w" ) ) )
      { fprintf( stderr, "Error in opening the file ... quit\n" ); exit( 1 ); }
 
      fprintf( ofp, "%s\n", "'DBLAT2.SUMM'     NAME OF SUMMARY OUTPUT FILE'"  );
      fprintf( ofp, "%s\n", "6                 UNIT NUMBER OF SUMMARY FILE'"  );
      fprintf( ofp, "%s\n", "'DBLAT2.SNAP'     NAME OF SNAPSHOT OUTPUT FILE'" );      fprintf( ofp, "%s\n",
      "-1                UNIT NUMBER OF SNAPSHOT FILE (NOT USED IF .LT. 0)" );
      fprintf( ofp, "%s\n",
      "F        LOGICAL FLAG, T TO REWIND SNAPSHOT FILE AFTER EACH RECORD." );
      fprintf( ofp, "%s\n", "F        LOGICAL FLAG, T TO STOP ON FAILURES." );
      fprintf( ofp, "%s\n", "T        LOGICAL FLAG, T TO TEST ERROR EXITS." );
      fprintf( ofp, "%s\n", "16.0     THRESHOLD VALUE OF TEST RATIO"        );
 
      fprintf( ofp, "%d%s\n", MAXNN, "                 NUMBER OF VALUES OF N" );
      for( i = 0; i < MAXNN; i++ ) fprintf( ofp,  "%d ", nval[ i ] );
      fprintf( ofp, "\t%s\n", "VALUES OF N" );
      fprintf( ofp, "%d%s\n", MAXNK, "                 NUMBER OF VALUES OF K" );
      for( i = 0; i < MAXNK; i++ ) fprintf( ofp,  "%d ", kval[ i ] );
      fprintf( ofp, "\t%s\n", "VALUES OF K" );
 
      fprintf( ofp, "%s\n",
      "4                 NUMBER OF VALUES OF INCX AND INCY" );
      fprintf( ofp, "%s\n", "1 2 -1 -2         VALUES OF INCX AND INCY" );
 
      fprintf( ofp, "%d%s\n", MAXAB,
      "                 NUMBER OF VALUES OF ALPHA" );
      for( i = 0; i < MAXAB; i++ ) fprintf( ofp,  "%6.3f ", alphaR[ i ] );
      fprintf( ofp, "\t%s\n", "VALUES OF ALPHA" );
      fprintf( ofp, "%d%s\n", MAXAB,
      "                 NUMBER OF VALUES OF BETA " );
      for( i = 0; i < MAXAB; i++ ) fprintf( ofp,  "%6.3f ", betaR [ i ] );
      fprintf( ofp, "\t%s\n", "VALUES OF BETA " );

      fprintf( ofp, "%s\n", "DGEMV  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "DGBMV  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "DSYMV  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "DSBMV  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "DSPMV  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "DTRMV  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "DTBMV  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "DTPMV  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "DTRSV  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "DTBSV  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "DTPSV  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "DGER   T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "DSYR   T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "DSPR   T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "DSYR2  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "DSPR2  T PUT F FOR NO TEST. SAME COLUMNS." );

      fclose( ofp );
/*
 * Single precision complex
 */
      strcpy( str, "cblat2_" );
      if( kfile >= 9 )
      { str[7]='0'+(kfile+1)/10; str[8]='0'+(kfile+1)%10; str[9]='\0';
      strcpy( &str[9], ".dat" ); }
      else { str[7] = '1' + kfile; str[8] = '\0'; strcpy( &str[8], ".dat" ); }
      if( !( ofp = fopen( str, "w" ) ) )
      { fprintf( stderr, "Error in opening the file ... quit\n" ); exit( 1 ); }
 
      fprintf( ofp, "%s\n", "'CBLAT2.SUMM'     NAME OF SUMMARY OUTPUT FILE'"  );
      fprintf( ofp, "%s\n", "6                 UNIT NUMBER OF SUMMARY FILE'"  );
      fprintf( ofp, "%s\n", "'CBLAT2.SNAP'     NAME OF SNAPSHOT OUTPUT FILE'" );
      fprintf( ofp, "%s\n",
      "-1                UNIT NUMBER OF SNAPSHOT FILE (NOT USED IF .LT. 0)" );
      fprintf( ofp, "%s\n",
      "F        LOGICAL FLAG, T TO REWIND SNAPSHOT FILE AFTER EACH RECORD." );
      fprintf( ofp, "%s\n", "F        LOGICAL FLAG, T TO STOP ON FAILURES." );
      fprintf( ofp, "%s\n", "T        LOGICAL FLAG, T TO TEST ERROR EXITS." );
      fprintf( ofp, "%s\n", "16.0     THRESHOLD VALUE OF TEST RATIO"        );
 
      fprintf( ofp, "%d%s\n", MAXNN, "                 NUMBER OF VALUES OF N" );
      for( i = 0; i < MAXNN; i++ ) fprintf( ofp,  "%d ", nval[ i ] );
      fprintf( ofp, "\t%s\n", "VALUES OF N" );
      fprintf( ofp, "%d%s\n", MAXNK, "                 NUMBER OF VALUES OF K" );
      for( i = 0; i < MAXNK; i++ ) fprintf( ofp,  "%d ", kval[ i ] );
      fprintf( ofp, "\t%s\n", "VALUES OF K" );
 
      fprintf( ofp, "%s\n",
      "4                 NUMBER OF VALUES OF INCX AND INCY" );
      fprintf( ofp, "%s\n", "1 2 -1 -2         VALUES OF INCX AND INCY" );
 
      fprintf( ofp, "%d%s\n", MAXAB,
      "                 NUMBER OF VALUES OF ALPHA" );
      for( i = 0; i < MAXAB; i++ )
      fprintf( ofp,  "(%6.3f,%6.3f) ", alphaR[ i ], alphaI[ i ] );
      fprintf( ofp, "\t%s\n", "VALUES OF ALPHA" );
      fprintf( ofp, "%d%s\n", MAXAB,
      "                 NUMBER OF VALUES OF BETA " );
      for( i = 0; i < MAXAB; i++ )
      fprintf( ofp,  "(%6.3f,%6.3f) ", betaR [ i ], betaI [ i ] );
      fprintf( ofp, "\t%s\n", "VALUES OF BETA " );

      fprintf( ofp, "%s\n", "CGEMV  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "CGBMV  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "CHEMV  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "CHBMV  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "CHPMV  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "CTRMV  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "CTBMV  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "CTPMV  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "CTRSV  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "CTBSV  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "CTPSV  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "CGERC  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "CGERU  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "CHER   T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "CHPR   T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "CHER2  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "CHPR2  T PUT F FOR NO TEST. SAME COLUMNS." );

      fclose( ofp );
/*
 * Double precision complex
 */
      strcpy( str, "zblat2_" );
      if( kfile >= 9 )
      { str[7]='0'+(kfile+1)/10; str[8]='0'+(kfile+1)%10; str[9]='\0';
      strcpy( &str[9], ".dat" ); }
      else { str[7] = '1' + kfile; str[8] = '\0'; strcpy( &str[8], ".dat" ); }
      if( !( ofp = fopen( str, "w" ) ) )
      { fprintf( stderr, "Error in opening the file ... quit\n" ); exit( 1 ); }
 
      fprintf( ofp, "%s\n", "'ZBLAT2.SUMM'     NAME OF SUMMARY OUTPUT FILE'"  );
      fprintf( ofp, "%s\n", "6                 UNIT NUMBER OF SUMMARY FILE'"  );
      fprintf( ofp, "%s\n", "'ZBLAT2.SNAP'     NAME OF SNAPSHOT OUTPUT FILE'" );
      fprintf( ofp, "%s\n",
      "-1                UNIT NUMBER OF SNAPSHOT FILE (NOT USED IF .LT. 0)" );
      fprintf( ofp, "%s\n",
      "F        LOGICAL FLAG, T TO REWIND SNAPSHOT FILE AFTER EACH RECORD." );
      fprintf( ofp, "%s\n", "F        LOGICAL FLAG, T TO STOP ON FAILURES." );
      fprintf( ofp, "%s\n", "T        LOGICAL FLAG, T TO TEST ERROR EXITS." );
      fprintf( ofp, "%s\n", "16.0     THRESHOLD VALUE OF TEST RATIO"        );
 
      fprintf( ofp, "%d%s\n", MAXNN, "                 NUMBER OF VALUES OF N" );
      for( i = 0; i < MAXNN; i++ ) fprintf( ofp,  "%d ", nval[ i ] );
      fprintf( ofp, "\t%s\n", "VALUES OF N" );
      fprintf( ofp, "%d%s\n", MAXNK, "                 NUMBER OF VALUES OF K" );
      for( i = 0; i < MAXNK; i++ ) fprintf( ofp,  "%d ", kval[ i ] );
      fprintf( ofp, "\t%s\n", "VALUES OF K" );
 
      fprintf( ofp, "%s\n",
      "4                 NUMBER OF VALUES OF INCX AND INCY" );
      fprintf( ofp, "%s\n", "1 2 -1 -2         VALUES OF INCX AND INCY" );
 
      fprintf( ofp, "%d%s\n", MAXAB,
      "                 NUMBER OF VALUES OF ALPHA" );
      for( i = 0; i < MAXAB; i++ )
      fprintf( ofp,  "(%6.3f,%6.3f) ", alphaR[ i ], alphaI[ i ] );
      fprintf( ofp, "\t%s\n", "VALUES OF ALPHA" );
      fprintf( ofp, "%d%s\n", MAXAB,
      "                 NUMBER OF VALUES OF BETA " );
      for( i = 0; i < MAXAB; i++ )
      fprintf( ofp,  "(%6.3f,%6.3f) ", betaR [ i ], betaI [ i ] );
      fprintf( ofp, "\t%s\n", "VALUES OF BETA " );

      fprintf( ofp, "%s\n", "ZGEMV  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "ZGBMV  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "ZHEMV  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "ZHBMV  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "ZHPMV  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "ZTRMV  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "ZTBMV  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "ZTPMV  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "ZTRSV  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "ZTBSV  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "ZTPSV  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "ZGERC  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "ZGERU  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "ZHER   T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "ZHPR   T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "ZHER2  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "ZHPR2  T PUT F FOR NO TEST. SAME COLUMNS." );

      fclose( ofp );
   }
   if( str ) free( str );
}
