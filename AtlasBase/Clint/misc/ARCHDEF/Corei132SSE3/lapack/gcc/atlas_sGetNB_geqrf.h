#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,160,400,800,1600,3280
 * N : 25,160,400,800,1600,3280
 * NB : 4,80,80,80,80,80
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 92) (nb_) = 4; \
   else (nb_) = 80; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
