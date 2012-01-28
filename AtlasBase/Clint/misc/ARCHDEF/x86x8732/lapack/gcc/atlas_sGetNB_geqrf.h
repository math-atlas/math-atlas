#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,96,168,240,312,624,1224,2424
 * N : 25,96,168,240,312,624,1224,2424
 * NB : 2,6,12,24,24,24,24,48
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 60) (nb_) = 2; \
   else if ((n_) < 132) (nb_) = 6; \
   else if ((n_) < 204) (nb_) = 12; \
   else if ((n_) < 1824) (nb_) = 24; \
   else (nb_) = 48; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
