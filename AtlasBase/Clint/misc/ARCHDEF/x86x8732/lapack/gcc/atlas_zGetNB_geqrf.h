#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,96,144,168,192,384,768,1512
 * N : 25,96,144,168,192,384,768,1512
 * NB : 4,12,12,12,24,24,24,24
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 60) (nb_) = 4; \
   else if ((n_) < 180) (nb_) = 12; \
   else (nb_) = 24; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
