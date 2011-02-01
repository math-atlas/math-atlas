#include "Fgendat.h"

main()
{
   FILE           * ofp;
   char           * str = NULL;
   int            i, itest, k, kfile;
   int            nval[MAXNN];
   float          alphaR[MAXAB], alphaI[MAXAB], betaR[MAXAB], betaI[MAXAB];

   str = (char * ) malloc( 13 );

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
      strcpy( str, "sblat3_" );
      if( kfile >= 9 )
      { str[7]='0'+(kfile+1)/10; str[8]='0'+(kfile+1)%10; str[9]='\0';
      strcpy( &str[9], ".dat" ); }
      else { str[7] = '1' + kfile; str[8] = '\0'; strcpy( &str[8], ".dat" ); }
      if( !( ofp = fopen( str, "w" ) ) )
      { fprintf( stderr, "Error in opening the file ... quit\n" ); exit( 1 ); }

      fprintf( ofp, "%s\n", "'SBLAT3.SUMM'     NAME OF SUMMARY OUTPUT FILE'"  );
      fprintf( ofp, "%s\n", "6                 UNIT NUMBER OF SUMMARY FILE'"  );
      fprintf( ofp, "%s\n", "'SBLAT3.SNAP'     NAME OF SNAPSHOT OUTPUT FILE'" );
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

      fprintf( ofp, "%d%s\n", MAXAB,
      "                 NUMBER OF VALUES OF ALPHA" );
      for( i = 0; i < MAXAB; i++ ) fprintf( ofp,  "%6.3f ", alphaR[ i ] );
      fprintf( ofp, "\t%s\n", "VALUES OF ALPHA" );
      fprintf( ofp, "%d%s\n", MAXAB,
      "                 NUMBER OF VALUES OF BETA " );
      for( i = 0; i < MAXAB; i++ ) fprintf( ofp,  "%6.3f ", betaR [ i ] );
      fprintf( ofp, "\t%s\n", "VALUES OF BETA " );

      fprintf( ofp, "%s\n", "SGEMM  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "SSYMM  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "STRMM  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "STRSM  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "SSYRK  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "SSYR2K T PUT F FOR NO TEST. SAME COLUMNS." );

      fclose( ofp );
/*
 * Double precision real
 */
      strcpy( str, "dblat3_" );
      if( kfile >= 9 )
      { str[7]='0'+(kfile+1)/10; str[8]='0'+(kfile+1)%10; str[9]='\0';
      strcpy( &str[9], ".dat" ); }
      else { str[7] = '1' + kfile; str[8] = '\0'; strcpy( &str[8], ".dat" ); }
      if( !( ofp = fopen( str, "w" ) ) )
      { fprintf( stderr, "Error in opening the file ... quit\n" ); exit( 1 ); }
 
      fprintf( ofp, "%s\n", "'DBLAT3.SUMM'     NAME OF SUMMARY OUTPUT FILE'"  );
      fprintf( ofp, "%s\n", "6                 UNIT NUMBER OF SUMMARY FILE'"  );
      fprintf( ofp, "%s\n", "'DBLAT3.SNAP'     NAME OF SNAPSHOT OUTPUT FILE'" );      fprintf( ofp, "%s\n",
      "-1                UNIT NUMBER OF SNAPSHOT FILE (NOT USED IF .LT. 0)" );
      fprintf( ofp, "%s\n",
      "F        LOGICAL FLAG, T TO REWIND SNAPSHOT FILE AFTER EACH RECORD." );
      fprintf( ofp, "%s\n", "F        LOGICAL FLAG, T TO STOP ON FAILURES." );
      fprintf( ofp, "%s\n", "T        LOGICAL FLAG, T TO TEST ERROR EXITS." );
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

      fprintf( ofp, "%s\n", "DGEMM  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "DSYMM  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "DTRMM  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "DTRSM  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "DSYRK  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "DSYR2K T PUT F FOR NO TEST. SAME COLUMNS." );

      fclose( ofp );
/*
 * Single precision complex
 */
      strcpy( str, "cblat3_" );
      if( kfile >= 9 )
      { str[7]='0'+(kfile+1)/10; str[8]='0'+(kfile+1)%10; str[9]='\0';
      strcpy( &str[9], ".dat" ); }
      else { str[7] = '1' + kfile; str[8] = '\0'; strcpy( &str[8], ".dat" ); }
      if( !( ofp = fopen( str, "w" ) ) )
      { fprintf( stderr, "Error in opening the file ... quit\n" ); exit( 1 ); }
 
      fprintf( ofp, "%s\n", "'CBLAT3.SUMM'     NAME OF SUMMARY OUTPUT FILE'"  );
      fprintf( ofp, "%s\n", "6                 UNIT NUMBER OF SUMMARY FILE'"  );
      fprintf( ofp, "%s\n", "'CBLAT3.SNAP'     NAME OF SNAPSHOT OUTPUT FILE'" );
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

      fprintf( ofp, "%s\n", "CGEMM  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "CHEMM  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "CSYMM  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "CTRMM  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "CTRSM  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "CHERK  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "CSYRK  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "CHER2K T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "CSYR2K T PUT F FOR NO TEST. SAME COLUMNS." );

      fclose( ofp );
/*
 * Double precision complex
 */
      strcpy( str, "zblat3_" );
      if( kfile >= 9 )
      { str[7]='0'+(kfile+1)/10; str[8]='0'+(kfile+1)%10; str[9]='\0';
      strcpy( &str[9], ".dat" ); }
      else { str[7] = '1' + kfile; str[8] = '\0'; strcpy( &str[8], ".dat" ); }
      if( !( ofp = fopen( str, "w" ) ) )
      { fprintf( stderr, "Error in opening the file ... quit\n" ); exit( 1 ); }
 
      fprintf( ofp, "%s\n", "'ZBLAT3.SUMM'     NAME OF SUMMARY OUTPUT FILE'"  );
      fprintf( ofp, "%s\n", "6                 UNIT NUMBER OF SUMMARY FILE'"  );
      fprintf( ofp, "%s\n", "'ZBLAT3.SNAP'     NAME OF SNAPSHOT OUTPUT FILE'" );
      fprintf( ofp, "%s\n",
      "-1                UNIT NUMBER OF SNAPSHOT FILE (NOT USED IF .LT. 0)" );
      fprintf( ofp, "%s\n",
      "F        LOGICAL FLAG, T TO REWIND SNAPSHOT FILE AFTER EACH RECORD." );
      fprintf( ofp, "%s\n", "F        LOGICAL FLAG, T TO STOP ON FAILURES." );
      fprintf( ofp, "%s\n", "T        LOGICAL FLAG, T TO TEST ERROR EXITS." );
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

      fprintf( ofp, "%s\n", "ZGEMM  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "ZHEMM  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "ZSYMM  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "ZTRMM  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "ZTRSM  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "ZHERK  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "ZSYRK  T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "ZHER2K T PUT F FOR NO TEST. SAME COLUMNS." );
      fprintf( ofp, "%s\n", "ZSYR2K T PUT F FOR NO TEST. SAME COLUMNS." );

      fclose( ofp );
   }
   if( str ) free( str );
}
