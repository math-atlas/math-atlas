#ifndef ATL_ctGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,96,240,480,1008,1536,1776,1920,2064
 * N : 25,96,240,480,1008,1536,1776,1920,2064
 * NB : 9,48,48,48,48,48,48,48,96
 */
#define ATL_ctGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 60) (nb_) = 9; \
   else if ((n_) < 1992) (nb_) = 48; \
   else (nb_) = 96; \
}


#endif    /* end ifndef ATL_ctGetNB_geqrf */
