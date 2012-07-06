#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,120,300,600,1260,2580,5160
 * N : 25,120,300,600,1260,2580,5160
 * NB : 2,60,60,60,60,60,120
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 72) (nb_) = 2; \
   else if ((n_) < 3870) (nb_) = 60; \
   else (nb_) = 120; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
