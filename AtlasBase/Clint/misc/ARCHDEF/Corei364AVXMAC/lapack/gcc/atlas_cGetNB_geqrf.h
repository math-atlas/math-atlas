#ifndef ATL_cGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,160,320,720,1440,2160,2960
 * N : 25,160,320,720,1440,2160,2960
 * NB : 5,80,80,80,80,80,160
 */
#define ATL_cGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 92) (nb_) = 5; \
   else if ((n_) < 2560) (nb_) = 80; \
   else (nb_) = 160; \
}


#endif    /* end ifndef ATL_cGetNB_geqrf */
