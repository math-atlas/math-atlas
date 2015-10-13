#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,108,180,216,252,540,1116
 * N : 25,108,180,216,252,540,1116
 * NB : 2,12,12,36,36,36,36
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 66) (nb_) = 2; \
   else if ((n_) < 198) (nb_) = 12; \
   else (nb_) = 36; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
