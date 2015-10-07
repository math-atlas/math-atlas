#ifndef ATL_cGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,67,110,195,366,708,1392,2760
 * N : 25,67,110,195,366,708,1392,2760
 * NB : 8,8,16,16,32,48,96,144
 */
#define ATL_cGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 88) (nb_) = 8; \
   else if ((n_) < 280) (nb_) = 16; \
   else if ((n_) < 537) (nb_) = 32; \
   else if ((n_) < 1050) (nb_) = 48; \
   else if ((n_) < 2076) (nb_) = 96; \
   else (nb_) = 144; \
}


#endif    /* end ifndef ATL_cGetNB_geqrf */
