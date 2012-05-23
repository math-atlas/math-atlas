#ifndef ATL_cGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,160,240,320,400,880,1760
 * N : 25,160,240,320,400,880,1760
 * NB : 4,12,24,80,80,80,80
 */
#define ATL_cGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 92) (nb_) = 4; \
   else if ((n_) < 200) (nb_) = 12; \
   else if ((n_) < 280) (nb_) = 24; \
   else (nb_) = 80; \
}


#endif    /* end ifndef ATL_cGetNB_geqrf */
