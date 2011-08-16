#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,144,336,672,1008,1392
 * N : 25,144,336,672,1008,1392
 * NB : 2,18,18,30,30,48
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 84) (nb_) = 2; \
   else if ((n_) < 504) (nb_) = 18; \
   else if ((n_) < 1200) (nb_) = 30; \
   else (nb_) = 48; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
