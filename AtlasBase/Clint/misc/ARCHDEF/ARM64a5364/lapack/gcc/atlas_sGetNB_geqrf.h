#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,240,528,1056,2160
 * N : 25,240,528,1056,2160
 * NB : 12,48,48,48,48
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 132) (nb_) = 12; \
   else (nb_) = 48; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
