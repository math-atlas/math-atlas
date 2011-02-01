#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,72,144,192,216,240,264,528,1032
 * N : 25,72,144,192,216,240,264,528,1032
 * NB : 4,8,8,8,8,24,24,24,24
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 48) (nb_) = 4; \
   else if ((n_) < 228) (nb_) = 8; \
   else (nb_) = 24; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
