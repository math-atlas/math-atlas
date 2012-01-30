#ifndef ATL_cGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,120,300,420,480,600,1260
 * N : 25,120,300,420,480,600,1260
 * NB : 5,12,12,12,60,60,60
 */
#define ATL_cGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 72) (nb_) = 5; \
   else if ((n_) < 450) (nb_) = 12; \
   else (nb_) = 60; \
}


#endif    /* end ifndef ATL_cGetNB_geqrf */
