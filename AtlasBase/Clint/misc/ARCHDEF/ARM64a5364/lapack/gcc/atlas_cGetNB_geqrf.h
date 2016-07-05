#ifndef ATL_cGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,144,192,240,288,624,1296
 * N : 25,144,192,240,288,624,1296
 * NB : 12,12,16,16,48,48,48
 */
#define ATL_cGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 168) (nb_) = 12; \
   else if ((n_) < 264) (nb_) = 16; \
   else (nb_) = 48; \
}


#endif    /* end ifndef ATL_cGetNB_geqrf */
