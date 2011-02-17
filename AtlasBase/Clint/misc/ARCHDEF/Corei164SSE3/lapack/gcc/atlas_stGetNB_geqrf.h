#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,144,288,648,1368,2016,2736,5544
 * N : 25,144,288,648,1368,2016,2736,5544
 * NB : 4,72,72,72,72,144,144,216
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 84) (nb_) = 4; \
   else if ((n_) < 1692) (nb_) = 72; \
   else if ((n_) < 4140) (nb_) = 144; \
   else (nb_) = 216; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
