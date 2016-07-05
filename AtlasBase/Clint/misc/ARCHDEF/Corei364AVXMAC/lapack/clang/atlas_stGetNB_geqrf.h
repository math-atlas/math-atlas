#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,240,320,400,560,1200,2400,2960,3600,4800,9680
 * N : 25,240,320,400,560,1200,2400,2960,3600,4800,9680
 * NB : 5,24,80,80,80,80,80,160,160,160,160
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 132) (nb_) = 5; \
   else if ((n_) < 280) (nb_) = 24; \
   else if ((n_) < 2680) (nb_) = 80; \
   else (nb_) = 160; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
