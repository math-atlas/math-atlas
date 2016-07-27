#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,160,240,320,720,1120,1280,1520,3120
 * N : 25,160,240,320,720,1120,1280,1520,3120
 * NB : 4,12,80,80,80,80,80,160,160
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 92) (nb_) = 4; \
   else if ((n_) < 200) (nb_) = 12; \
   else if ((n_) < 1400) (nb_) = 80; \
   else (nb_) = 160; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
