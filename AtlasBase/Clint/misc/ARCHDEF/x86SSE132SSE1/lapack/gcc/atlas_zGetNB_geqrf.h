#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,84,168,378,546,630,756,1512
 * N : 25,84,168,378,546,630,756,1512
 * NB : 4,6,12,12,18,42,42,42
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 54) (nb_) = 4; \
   else if ((n_) < 126) (nb_) = 6; \
   else if ((n_) < 462) (nb_) = 12; \
   else if ((n_) < 588) (nb_) = 18; \
   else (nb_) = 42; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
