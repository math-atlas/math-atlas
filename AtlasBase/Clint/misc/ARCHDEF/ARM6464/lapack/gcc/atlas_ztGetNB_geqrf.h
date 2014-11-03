#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,100,175,326,628,930,1232,1836,2440
 * N : 25,100,175,326,628,930,1232,1836,2440
 * NB : 8,10,10,10,10,14,24,104,104
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 62) (nb_) = 8; \
   else if ((n_) < 779) (nb_) = 10; \
   else if ((n_) < 1081) (nb_) = 14; \
   else if ((n_) < 1534) (nb_) = 24; \
   else (nb_) = 104; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
