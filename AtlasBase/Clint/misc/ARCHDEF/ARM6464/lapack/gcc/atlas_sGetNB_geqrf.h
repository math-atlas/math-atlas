#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,145,205,266,326,387,447,508,992,1960
 * N : 25,145,205,266,326,387,447,508,992,1960
 * NB : 12,12,24,24,24,44,48,48,48,96
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 175) (nb_) = 12; \
   else if ((n_) < 356) (nb_) = 24; \
   else if ((n_) < 417) (nb_) = 44; \
   else if ((n_) < 1476) (nb_) = 48; \
   else (nb_) = 96; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
