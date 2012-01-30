#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,180,240,300,360,720,1500,3060
 * N : 25,180,240,300,360,720,1500,3060
 * NB : 4,12,12,60,60,60,60,120
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 102) (nb_) = 4; \
   else if ((n_) < 270) (nb_) = 12; \
   else if ((n_) < 2280) (nb_) = 60; \
   else (nb_) = 120; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
