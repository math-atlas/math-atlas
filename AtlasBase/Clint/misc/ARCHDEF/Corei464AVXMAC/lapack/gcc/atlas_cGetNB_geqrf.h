#ifndef ATL_cGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,160,400,800,1680,3360
 * N : 25,160,400,800,1680,3360
 * NB : 9,80,80,80,80,160
 */
#define ATL_cGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 92) (nb_) = 9; \
   else if ((n_) < 2520) (nb_) = 80; \
   else (nb_) = 160; \
}


#endif    /* end ifndef ATL_cGetNB_geqrf */
