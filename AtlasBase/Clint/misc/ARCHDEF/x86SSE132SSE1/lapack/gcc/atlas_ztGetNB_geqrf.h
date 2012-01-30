#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,126,294,336,420,588,1218,2436
 * N : 25,126,294,336,420,588,1218,2436
 * NB : 9,42,18,42,42,42,42,42
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 75) (nb_) = 9; \
   else if ((n_) < 210) (nb_) = 42; \
   else if ((n_) < 315) (nb_) = 18; \
   else (nb_) = 42; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
