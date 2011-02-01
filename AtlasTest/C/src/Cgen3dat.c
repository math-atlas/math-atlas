#include "Cgendat.h"

main()
{
   FILE           * ofp;
   char           * str = NULL;
   int            i, itest, k, kfile;
   int            nval[MAXNN];
   float          alphaR[MAXAB], alphaI[MAXAB], betaR[MAXAB], betaI[MAXAB];

   str = (char * ) malloc( 15 );

   srand48( (long)( time( NULL ) ) );

   for( kfile = 0; kfile < MAXFILES; kfile++ )
   {
      for( i = 0; i < MAXNN; i++ ) nval[i] = lrand48() % MAXN3;

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
      strcpy( str, "c_sblat3_" );
      if( kfile >= 9 )
      { str[9]='0'+(kfile+1)/10; str[10]='0'+(kfile+1)%10; str[11]='\0';
      strcpy( &str[11], ".dat" ); }
      else { str[9] = '1' + kfile; str[10] = '\0'; strcpy( &str[10], ".dat" ); }
      if( !( ofp = fopen( str, "w" ) ) )
      { fprintf( stderr, "Error in opening the file ... quit\n" ); exit( 1 ); }

      fprintf( ofp, "%s\n", "'SBLAT3.SNAP'     NAME OF SNAPSHOT OUTPUT FILE'" );
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

      fprintf( ofp, "%d%s\n", MAXAB,
      "                 NUMBER OF VALUES OF ALPHA" );
      for( i = 0; i < MAXAB; i++ ) fprintf( ofp,  "%6.3f ", alphaR[ i ] );
      fprintf( ofp, "\t%s\n", "VALUES OF ALPHA" );
      fprintf( ofp, "%d%s\n", MAXAB,
      "                 NUMBER OF VALUES OF BETA " );
      for( i = 0; i < MAXAB; i++ ) fprintf( ofp,  "%6.3f ", betaR [ i ] );
      fprintf( ofp, "\t%s\n", "VALUES OF BETA " );

      fprintf( ofp, "%s\n", "cblas_sgemm  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_ssymm  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_strmm  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_strsm  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_ssyrk  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_ssyr2k T PUT F FOR NO TEST. SAME COLUMNS." );

      fclose( ofp );
/*
 * Double precision real
 */
      strcpy( str, "c_dblat3_" );
      if( kfile >= 9 )
      { str[9]='0'+(kfile+1)/10; str[10]='0'+(kfile+1)%10; str[11]='\0';
      strcpy( &str[11], ".dat" ); }
      else { str[9] = '1' + kfile; str[10] = '\0'; strcpy( &str[10], ".dat" ); }
      if( !( ofp = fopen( str, "w" ) ) )
      { fprintf( stderr, "Error in opening the file ... quit\n" ); exit( 1 ); }
 
      fprintf( ofp, "%s\n", "'DBLAT3.SNAP'     NAME OF SNAPSHOT OUTPUT FILE'" );      fprintf( ofp, "%s\n",
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
 
      fprintf( ofp, "%d%s\n", MAXAB,
      "                 NUMBER OF VALUES OF ALPHA" );
      for( i = 0; i < MAXAB; i++ ) fprintf( ofp,  "%6.3f ", alphaR[ i ] );
      fprintf( ofp, "\t%s\n", "VALUES OF ALPHA" );
      fprintf( ofp, "%d%s\n", MAXAB,
      "                 NUMBER OF VALUES OF BETA " );
      for( i = 0; i < MAXAB; i++ ) fprintf( ofp,  "%6.3f ", betaR [ i ] );
      fprintf( ofp, "\t%s\n", "VALUES OF BETA " );

      fprintf( ofp, "%s\n", "cblas_dgemm  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_dsymm  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_dtrmm  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_dtrsm  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_dsyrk  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_dsyr2k T PUT F FOR NO TEST. SAME COLUMNS." );

      fclose( ofp );
/*
 * Single precision complex
 */
      strcpy( str, "c_cblat3_" );
      if( kfile >= 9 )
      { str[9]='0'+(kfile+1)/10; str[10]='0'+(kfile+1)%10; str[11]='\0';
      strcpy( &str[11], ".dat" ); }
      else { str[9] = '1' + kfile; str[10] = '\0'; strcpy( &str[10], ".dat" ); }
      if( !( ofp = fopen( str, "w" ) ) )
      { fprintf( stderr, "Error in opening the file ... quit\n" ); exit( 1 ); }
 
      fprintf( ofp, "%s\n", "'CBLAT3.SNAP'     NAME OF SNAPSHOT OUTPUT FILE'" );
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

      fprintf( ofp, "%s\n", "cblas_cgemm  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_chemm  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_csymm  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_ctrmm  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_ctrsm  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_cherk  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_csyrk  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_cher2k T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_csyr2k T PUT F FOR NO TEST. SAME COLUMNS." );

      fclose( ofp );
/*
 * Double precision complex
 */
      strcpy( str, "c_zblat3_" );
      if( kfile >= 9 )
      { str[9]='0'+(kfile+1)/10; str[10]='0'+(kfile+1)%10; str[11]='\0';
      strcpy( &str[11], ".dat" ); }
      else { str[9] = '1' + kfile; str[10] = '\0'; strcpy( &str[10], ".dat" ); }
      if( !( ofp = fopen( str, "w" ) ) )
      { fprintf( stderr, "Error in opening the file ... quit\n" ); exit( 1 ); }
 
      fprintf( ofp, "%s\n", "'ZBLAT3.SNAP'     NAME OF SNAPSHOT OUTPUT FILE'" );
      fprintf( ofp, "%s\n",
      "-1                UNIT NUMBER OF SNAPSHOT FILE (NOT USED IF .LT. 0)" );
      fprintf( ofp, "%s\n",
      "F        LOGICAL FLAG, T TO REWIND SNAPSHOT FILE AFTER EACH RECORD." );
      fprintf( ofp, "%s\n", "F        LOGICAL FLAG, T TO STOP ON FAILURES." );
      fprintf( ofp, "%s\n", "T        LOGICAL FLAG, T TO TEST ERROR EXITS." );
      fprintf( ofp, "%s\n",
      "2        0 TO TEST COLUMN-MAJOR, 1 TO TEST ROW-MAJOR, 2 TO TEST BOTH" );
      fprintf( ofp, "%s\n", "16.0     THRESHOLD VALUE OF TEST RATIO"        );
 
      fprintf( ofp, "%2d%s\n", MAXNN, "                NUMBER OF VALUES OF N" );      for( i = 0; i < MAXNN; i++ ) fprintf( ofp,  "%d ", nval[ i ] );
      fprintf( ofp, "\t%s\n", "VALUES OF N" );
 
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

      fprintf( ofp, "%s\n", "cblas_zgemm  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_zhemm  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_zsymm  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_ztrmm  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_ztrsm  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_zherk  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_zsyrk  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_zher2k T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "cblas_zsyr2k T PUT F FOR NO TEST. SAME COLUMNS." );

      fclose( ofp );
   }
   if( str ) free( str );
}
