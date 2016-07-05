#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,120,180,240,300,660,1380,2820
 * N : 25,120,180,240,300,660,1380,2820
 * NB : 9,4,12,60,60,60,60,120
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 72) (nb_) = 9; \
   else if ((n_) < 150) (nb_) = 4; \
   else if ((n_) < 210) (nb_) = 12; \
   else if ((n_) < 2100) (nb_) = 60; \
   else (nb_) = 120; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
