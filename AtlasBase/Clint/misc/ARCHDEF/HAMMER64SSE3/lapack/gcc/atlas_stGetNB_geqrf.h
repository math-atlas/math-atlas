#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,168,336,756,1596,3192
 * N : 25,168,336,756,1596,3192
 * NB : 4,84,84,84,84,84
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 96) (nb_) = 4; \
   else (nb_) = 84; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
