#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,72,108,144,324,648,1296,2628
 * N : 25,72,108,144,324,648,1296,2628
 * NB : 4,12,12,36,36,36,36,36
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 48) (nb_) = 4; \
   else if ((n_) < 126) (nb_) = 12; \
   else (nb_) = 36; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
