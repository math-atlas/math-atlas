#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,72,120,240,456,912
 * N : 25,72,120,240,456,912
 * NB : 5,24,24,24,24,48
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 48) (nb_) = 5; \
   else if ((n_) < 684) (nb_) = 24; \
   else (nb_) = 48; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
