#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,108,216,252,288,324,432,864
 * N : 25,108,216,252,288,324,432,864
 * NB : 3,12,12,12,36,36,36,72
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 66) (nb_) = 3; \
   else if ((n_) < 270) (nb_) = 12; \
   else if ((n_) < 648) (nb_) = 36; \
   else (nb_) = 72; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
