#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,144,216,288,432,504,576,648,1368
 * N : 25,144,216,288,432,504,576,648,1368
 * NB : 2,12,16,16,16,24,72,72,72
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 84) (nb_) = 2; \
   else if ((n_) < 180) (nb_) = 12; \
   else if ((n_) < 468) (nb_) = 16; \
   else if ((n_) < 540) (nb_) = 24; \
   else (nb_) = 72; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
