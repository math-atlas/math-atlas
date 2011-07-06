#ifndef ATL_ctGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,160,400,800,1600,3280
 * N : 25,160,400,800,1600,3280
 * NB : 11,80,80,80,80,80
 */
#define ATL_ctGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 92) (nb_) = 11; \
   else (nb_) = 80; \
}


#endif    /* end ifndef ATL_ctGetNB_geqrf */
