#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,180,420,840,1680,3360
 * N : 25,180,420,840,1680,3360
 * NB : 4,60,60,60,120,120
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 102) (nb_) = 4; \
   else if ((n_) < 1260) (nb_) = 60; \
   else (nb_) = 120; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
