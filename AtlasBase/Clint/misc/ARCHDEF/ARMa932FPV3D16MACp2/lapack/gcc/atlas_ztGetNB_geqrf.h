#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,72,96,120,168,192,216,432,864
 * N : 25,72,96,120,168,192,216,432,864
 * NB : 4,6,12,12,12,24,24,24,24
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 48) (nb_) = 4; \
   else if ((n_) < 84) (nb_) = 6; \
   else if ((n_) < 180) (nb_) = 12; \
   else (nb_) = 24; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
