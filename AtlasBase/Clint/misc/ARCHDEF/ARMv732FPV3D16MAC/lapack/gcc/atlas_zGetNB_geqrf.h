#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,96,120,144,168,192,360,696
 * N : 25,96,120,144,168,192,360,696
 * NB : 4,6,12,12,24,24,24,24
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 60) (nb_) = 4; \
   else if ((n_) < 108) (nb_) = 6; \
   else if ((n_) < 156) (nb_) = 12; \
   else (nb_) = 24; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
