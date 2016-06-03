#ifndef ATL_ctGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,160,320,640,1360,1680,2000,2720,5520
 * N : 25,160,320,640,1360,1680,2000,2720,5520
 * NB : 9,80,80,80,80,80,160,160,160
 */
#define ATL_ctGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 92) (nb_) = 9; \
   else if ((n_) < 1840) (nb_) = 80; \
   else (nb_) = 160; \
}


#endif    /* end ifndef ATL_ctGetNB_geqrf */
