#ifndef ATL_ctGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,120,180,300,600,1260,2520
 * N : 25,120,180,300,600,1260,2520
 * NB : 8,6,60,60,60,120,120
 */
#define ATL_ctGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 72) (nb_) = 8; \
   else if ((n_) < 150) (nb_) = 6; \
   else if ((n_) < 930) (nb_) = 60; \
   else (nb_) = 120; \
}


#endif    /* end ifndef ATL_ctGetNB_geqrf */
