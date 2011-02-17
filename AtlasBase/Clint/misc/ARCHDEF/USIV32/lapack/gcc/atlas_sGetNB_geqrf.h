#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,160,240,320,400,800,1200,1440,1680
 * N : 25,160,240,320,400,800,1200,1440,1680
 * NB : 1,12,16,16,28,32,32,80,80
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 92) (nb_) = 1; \
   else if ((n_) < 200) (nb_) = 12; \
   else if ((n_) < 360) (nb_) = 16; \
   else if ((n_) < 600) (nb_) = 28; \
   else if ((n_) < 1320) (nb_) = 32; \
   else (nb_) = 80; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
