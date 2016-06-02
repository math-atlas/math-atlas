#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,216,288,360,504,1080,2232,4536
 * N : 25,216,288,360,504,1080,2232,4536
 * NB : 9,12,24,24,36,36,36,48
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 120) (nb_) = 9; \
   else if ((n_) < 252) (nb_) = 12; \
   else if ((n_) < 432) (nb_) = 24; \
   else if ((n_) < 3384) (nb_) = 36; \
   else (nb_) = 48; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
