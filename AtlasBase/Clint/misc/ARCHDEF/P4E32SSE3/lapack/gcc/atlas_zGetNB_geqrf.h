#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,72,108,144,324,684,1368
 * N : 25,72,108,144,324,684,1368
 * NB : 9,12,36,36,36,36,36
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 48) (nb_) = 9; \
   else if ((n_) < 90) (nb_) = 12; \
   else (nb_) = 36; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
