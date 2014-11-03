#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,86,147,270,516,1008,1254,1500,1746,1992,3960
 * N : 25,86,147,270,516,1008,1254,1500,1746,1992,3960
 * NB : 12,12,24,24,24,24,24,36,36,144,144
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 116) (nb_) = 12; \
   else if ((n_) < 1377) (nb_) = 24; \
   else if ((n_) < 1869) (nb_) = 36; \
   else (nb_) = 144; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
