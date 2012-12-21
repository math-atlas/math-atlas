#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,216,432,936,1440,1944,3960
 * N : 25,216,432,936,1440,1944,3960
 * NB : 1,24,72,72,72,216,216
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 120) (nb_) = 1; \
   else if ((n_) < 324) (nb_) = 24; \
   else if ((n_) < 1692) (nb_) = 72; \
   else (nb_) = 216; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
