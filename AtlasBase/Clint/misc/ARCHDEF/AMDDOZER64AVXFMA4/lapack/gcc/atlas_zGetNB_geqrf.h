#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,156,234,312,390,858,1716
 * N : 25,156,234,312,390,858,1716
 * NB : 4,12,12,78,78,78,78
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 90) (nb_) = 4; \
   else if ((n_) < 273) (nb_) = 12; \
   else (nb_) = 78; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
