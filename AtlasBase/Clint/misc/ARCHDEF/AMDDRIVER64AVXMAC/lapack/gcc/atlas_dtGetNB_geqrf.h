#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,152,228,304,380,608,684,760,836,912,1064,1140,1216,1292,1368,1520,1748,3572,7144
 * N : 25,152,228,304,380,608,684,760,836,912,1064,1140,1216,1292,1368,1520,1748,3572,7144
 * NB : 4,12,20,76,76,76,76,92,96,76,104,104,104,112,160,152,152,152,152
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 88) (nb_) = 4; \
   else if ((n_) < 190) (nb_) = 12; \
   else if ((n_) < 266) (nb_) = 20; \
   else if ((n_) < 722) (nb_) = 76; \
   else if ((n_) < 798) (nb_) = 92; \
   else if ((n_) < 874) (nb_) = 96; \
   else if ((n_) < 988) (nb_) = 76; \
   else if ((n_) < 1254) (nb_) = 104; \
   else if ((n_) < 1330) (nb_) = 112; \
   else if ((n_) < 1444) (nb_) = 160; \
   else (nb_) = 152; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
