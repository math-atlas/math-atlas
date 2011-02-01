#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,96,168,240,336,408,456,480,504,672,1344,2664,5304
 * N : 25,96,168,240,336,408,456,480,504,672,1344,2664,5304
 * NB : 4,8,8,8,16,16,16,24,96,120,120,120,120
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 60) (nb_) = 4; \
   else if ((n_) < 288) (nb_) = 8; \
   else if ((n_) < 468) (nb_) = 16; \
   else if ((n_) < 492) (nb_) = 24; \
   else if ((n_) < 588) (nb_) = 96; \
   else (nb_) = 120; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
