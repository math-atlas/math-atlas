#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,120,180,300,660,1380,2820
 * N : 25,120,180,300,660,1380,2820
 * NB : 4,4,60,60,60,60,120
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 150) (nb_) = 4; \
   else if ((n_) < 2100) (nb_) = 60; \
   else (nb_) = 120; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
