#ifndef ATL_ctGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,144,288,648,1008,1152,1224,1296,1368,2808
 * N : 25,144,288,648,1008,1152,1224,1296,1368,2808
 * NB : 4,12,12,12,12,12,12,12,72,72
 */
#define ATL_ctGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 84) (nb_) = 4; \
   else if ((n_) < 1332) (nb_) = 12; \
   else (nb_) = 72; \
}


#endif    /* end ifndef ATL_ctGetNB_geqrf */
