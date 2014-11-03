#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,83,142,260,496,968,1086,1204,1440,1912,3800
 * N : 25,83,142,260,496,968,1086,1204,1440,1912,3800
 * NB : 8,12,16,24,24,24,80,80,80,80,80
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 54) (nb_) = 8; \
   else if ((n_) < 112) (nb_) = 12; \
   else if ((n_) < 201) (nb_) = 16; \
   else if ((n_) < 1027) (nb_) = 24; \
   else (nb_) = 80; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
