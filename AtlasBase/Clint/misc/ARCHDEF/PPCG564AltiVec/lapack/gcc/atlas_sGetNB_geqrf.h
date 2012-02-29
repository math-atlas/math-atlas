#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,160,240,320,720,1440,2960
 * N : 25,160,240,320,720,1440,2960
 * NB : 2,12,12,80,80,80,160
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 92) (nb_) = 2; \
   else if ((n_) < 280) (nb_) = 12; \
   else if ((n_) < 2200) (nb_) = 80; \
   else (nb_) = 160; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
