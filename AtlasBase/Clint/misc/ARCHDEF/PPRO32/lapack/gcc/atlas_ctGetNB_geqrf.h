#ifndef ATL_ctGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,96,168,312,600
 * N : 25,96,168,312,600
 * NB : 3,24,24,24,24
 */
#define ATL_ctGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 60) (nb_) = 3; \
   else (nb_) = 24; \
}


#endif    /* end ifndef ATL_ctGetNB_geqrf */
