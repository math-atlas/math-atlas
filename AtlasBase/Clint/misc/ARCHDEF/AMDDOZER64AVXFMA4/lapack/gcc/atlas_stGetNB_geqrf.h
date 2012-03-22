#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,180,420,840,1680,3420,6840
 * N : 25,180,420,840,1680,3420,6840
 * NB : 2,60,60,60,60,60,120
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 102) (nb_) = 2; \
   else if ((n_) < 5130) (nb_) = 60; \
   else (nb_) = 120; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
