#ifndef ATL_ctGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,96,168,312,624,1224,2424
 * N : 25,96,168,312,624,1224,2424
 * NB : 4,36,24,24,24,24,24
 */
#define ATL_ctGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 60) (nb_) = 4; \
   else if ((n_) < 132) (nb_) = 36; \
   else (nb_) = 24; \
}


#endif    /* end ifndef ATL_ctGetNB_geqrf */
