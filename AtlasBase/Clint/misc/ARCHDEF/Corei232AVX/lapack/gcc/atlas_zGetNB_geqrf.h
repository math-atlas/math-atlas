#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,180,420,900,1860
 * N : 25,180,420,900,1860
 * NB : 5,12,60,60,60
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 102) (nb_) = 5; \
   else if ((n_) < 300) (nb_) = 12; \
   else (nb_) = 60; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
