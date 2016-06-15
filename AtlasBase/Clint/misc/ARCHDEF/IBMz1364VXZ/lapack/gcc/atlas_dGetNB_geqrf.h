#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,160,320,640,1360,2800
 * N : 25,160,320,640,1360,2800
 * NB : 12,80,24,16,80,80
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 92) (nb_) = 12; \
   else if ((n_) < 240) (nb_) = 80; \
   else if ((n_) < 480) (nb_) = 24; \
   else if ((n_) < 1000) (nb_) = 16; \
   else (nb_) = 80; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
