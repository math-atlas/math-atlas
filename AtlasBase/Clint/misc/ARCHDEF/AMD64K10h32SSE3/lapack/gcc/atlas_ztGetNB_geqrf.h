#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,132,198,264,594,1254,2574
 * N : 25,132,198,264,594,1254,2574
 * NB : 12,24,66,66,66,66,66
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 78) (nb_) = 12; \
   else if ((n_) < 165) (nb_) = 24; \
   else (nb_) = 66; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
