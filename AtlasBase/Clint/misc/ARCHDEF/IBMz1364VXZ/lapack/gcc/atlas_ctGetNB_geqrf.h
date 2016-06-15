#ifndef ATL_ctGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,240,480,1040,1520,1760,2080
 * N : 25,240,480,1040,1520,1760,2080
 * NB : 4,80,80,80,80,160,160
 */
#define ATL_ctGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 132) (nb_) = 4; \
   else if ((n_) < 1640) (nb_) = 80; \
   else (nb_) = 160; \
}


#endif    /* end ifndef ATL_ctGetNB_geqrf */
