#include "Cgendat.h"

main()
{
   FILE           * ofp;
   char           * str = NULL;
   int            i, itest, k, kfile;
   int            kval[MAXNK], nval[MAXNN];
   float          alphaR[MAXAB], alphaI[MAXAB], betaR[MAXAB], betaI[MAXAB];

   str = (char * ) malloc( 16 );

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
      strcpy( str, "c_sblat2_" );
      if( kfile >= 9 )
      { str[9]='0'+(kfile+1)/10; str[10]='0'+(kfile+1)%10; str[11]='\0';
      strcpy( &str[11], ".dat" ); }
      else { str[9] = '1' + kfile; str[10] = '\0'; strcpy( &str[10], ".dat" ); }
      if( !( ofp = fopen( str, "w" ) ) )
      { fprintf( stderr, "Error in opening the file ... quit\n" ); exit( 1 ); }

      fprintf( ofp, "%s\n", "'SBLAT2.SNAP'     NAME OF SNAPSHOT OUTPUT FILE'" );
      fprintf( ofp, "%s\n",
      "-1                UNIT NUMBER OF SNAPSHOT FILE (NOT USED IF .LT. 0)" );
      fprintf( ofp, "%s\n",
      "F        LOGICAL FLAG, T TO REWIND SNAPSHOT FILE AFTER EACH RECORD." );
      fprintf( ofp, "%s\n", "F        LOGICAL FLAG, T TO STOP ON FAILURES." );
      fprintf( ofp, "%s\n", "T        LOGICAL FLAG, T TO TEST ERROR EXITS." );
      fprintf( ofp, "%s\n",
      "2        0 TO TEST COLUMN-MAJOR, 1 TO TEST ROW-MAJOR, 2 TO TEST BOTH" );
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

      fprintf( ofp, "%s\n", "cblas_sgemv  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_sgbmv  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_ssymv  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_ssbmv  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_sspmv  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_strmv  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_stbmv  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_stpmv  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_strsv  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_stbsv  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_stpsv  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_sger   T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_ssyr   T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_sspr   T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_ssyr2  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_sspr2  T PUT F FOR NO TEST. SAME COLUMNS." );

      fclose( ofp );
/*
 * Double precision real
 */
      strcpy( str, "c_dblat2_" );
      if( kfile >= 9 )
      { str[9]='0'+(kfile+1)/10; str[10]='0'+(kfile+1)%10; str[11]='\0';
      strcpy( &str[11], ".dat" ); }
      else { str[9] = '1' + kfile; str[10] = '\0'; strcpy( &str[10], ".dat" ); }
      if( !( ofp = fopen( str, "w" ) ) )
      { fprintf( stderr, "Error in opening the file ... quit\n" ); exit( 1 ); }
 
      fprintf( ofp, "%s\n", "'DBLAT2.SNAP'     NAME OF SNAPSHOT OUTPUT FILE'" );      fprintf( ofp, "%s\n",
      "-1                UNIT NUMBER OF SNAPSHOT FILE (NOT USED IF .LT. 0)" );
      fprintf( ofp, "%s\n",
      "F        LOGICAL FLAG, T TO REWIND SNAPSHOT FILE AFTER EACH RECORD." );
      fprintf( ofp, "%s\n", "F        LOGICAL FLAG, T TO STOP ON FAILURES." );
      fprintf( ofp, "%s\n", "T        LOGICAL FLAG, T TO TEST ERROR EXITS." );
      fprintf( ofp, "%s\n",
      "2        0 TO TEST COLUMN-MAJOR, 1 TO TEST ROW-MAJOR, 2 TO TEST BOTH" );
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

      fprintf( ofp, "%s\n", "cblas_dgemv  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_dgbmv  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_dsymv  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_dsbmv  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_dspmv  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_dtrmv  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_dtbmv  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_dtpmv  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_dtrsv  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_dtbsv  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_dtpsv  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_dger   T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_dsyr   T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_dspr   T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_dsyr2  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_dspr2  T PUT F FOR NO TEST. SAME COLUMNS." );

      fclose( ofp );
/*
 * Single precision complex
 */
      strcpy( str, "c_cblat2_" );
      if( kfile >= 9 )
      { str[9]='0'+(kfile+1)/10; str[10]='0'+(kfile+1)%10; str[11]='\0';
      strcpy( &str[11], ".dat" ); }
      else { str[9] = '1' + kfile; str[10] = '\0'; strcpy( &str[10], ".dat" ); }
      if( !( ofp = fopen( str, "w" ) ) )
      { fprintf( stderr, "Error in opening the file ... quit\n" ); exit( 1 ); }
 
      fprintf( ofp, "%s\n", "'CBLAT2.SNAP'     NAME OF SNAPSHOT OUTPUT FILE'" );
      fprintf( ofp, "%s\n",
      "-1                UNIT NUMBER OF SNAPSHOT FILE (NOT USED IF .LT. 0)" );
      fprintf( ofp, "%s\n",
      "F        LOGICAL FLAG, T TO REWIND SNAPSHOT FILE AFTER EACH RECORD." );
      fprintf( ofp, "%s\n", "F        LOGICAL FLAG, T TO STOP ON FAILURES." );
      fprintf( ofp, "%s\n", "T        LOGICAL FLAG, T TO TEST ERROR EXITS." );
      fprintf( ofp, "%s\n",
      "2        0 TO TEST COLUMN-MAJOR, 1 TO TEST ROW-MAJOR, 2 TO TEST BOTH" );
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

      fprintf( ofp, "%s\n", "cblas_cgemv  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_cgbmv  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_chemv  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_chbmv  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_chpmv  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_ctrmv  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_ctbmv  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_ctpmv  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_ctrsv  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_ctbsv  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_ctpsv  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_cgerc  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_cgeru  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_cher   T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_chpr   T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_cher2  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_chpr2  T PUT F FOR NO TEST. SAME COLUMNS." );

      fclose( ofp );
/*
 * Double precision complex
 */
      strcpy( str, "c_zblat2_" );
      if( kfile >= 9 )
      { str[9]='0'+(kfile+1)/10; str[10]='0'+(kfile+1)%10; str[11]='\0';
      strcpy( &str[11], ".dat" ); }
      else { str[9] = '1' + kfile; str[10] = '\0'; strcpy( &str[10], ".dat" ); }
      if( !( ofp = fopen( str, "w" ) ) )
      { fprintf( stderr, "Error in opening the file ... quit\n" ); exit( 1 ); }
 
      fprintf( ofp, "%s\n", "'ZBLAT2.SNAP'     NAME OF SNAPSHOT OUTPUT FILE'" );
      fprintf( ofp, "%s\n",
      "-1                UNIT NUMBER OF SNAPSHOT FILE (NOT USED IF .LT. 0)" );
      fprintf( ofp, "%s\n",
      "F        LOGICAL FLAG, T TO REWIND SNAPSHOT FILE AFTER EACH RECORD." );
      fprintf( ofp, "%s\n", "F        LOGICAL FLAG, T TO STOP ON FAILURES." );
      fprintf( ofp, "%s\n", "T        LOGICAL FLAG, T TO TEST ERROR EXITS." );
      fprintf( ofp, "%s\n",
      "2        0 TO TEST COLUMN-MAJOR, 1 TO TEST ROW-MAJOR, 2 TO TEST BOTH" );
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

      fprintf( ofp, "%s\n", "cblas_zgemv  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_zgbmv  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_zhemv  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_zhbmv  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_zhpmv  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_ztrmv  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_ztbmv  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_ztpmv  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_ztrsv  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_ztbsv  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_ztpsv  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_zgerc  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_zgeru  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_zher   T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_zhpr   T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_zher2  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_zhpr2  T PUT F FOR NO TEST. SAME COLUMNS." );

      fclose( ofp );
   }
   if( str ) free( str );
}
