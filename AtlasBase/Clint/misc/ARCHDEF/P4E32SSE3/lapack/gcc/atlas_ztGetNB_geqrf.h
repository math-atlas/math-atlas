#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,108,216,432,864,1728
 * N : 25,108,216,432,864,1728
 * NB : 9,36,36,36,36,36
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 66) (nb_) = 9; \
   else (nb_) = 36; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
