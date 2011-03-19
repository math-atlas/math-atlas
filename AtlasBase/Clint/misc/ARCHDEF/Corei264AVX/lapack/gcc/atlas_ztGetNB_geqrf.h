#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,84,196,392,812,1624,2436,2856,3052,3164,3192,3220,3248,3276
 * N : 25,84,196,392,812,1624,2436,2856,3052,3164,3192,3220,3248,3276
 * NB : 9,28,28,28,40,28,56,56,56,56,64,80,56,96
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 54) (nb_) = 9; \
   else if ((n_) < 602) (nb_) = 28; \
   else if ((n_) < 1218) (nb_) = 40; \
   else if ((n_) < 2030) (nb_) = 28; \
   else if ((n_) < 3178) (nb_) = 56; \
   else if ((n_) < 3206) (nb_) = 64; \
   else if ((n_) < 3234) (nb_) = 80; \
   else if ((n_) < 3262) (nb_) = 56; \
   else (nb_) = 96; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
