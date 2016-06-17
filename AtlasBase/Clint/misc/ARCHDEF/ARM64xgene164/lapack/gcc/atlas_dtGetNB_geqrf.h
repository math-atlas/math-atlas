#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,96,144,192,240,336,384,480,960,1968,3984
 * N : 25,96,144,192,240,336,384,480,960,1968,3984
 * NB : 1,12,12,20,20,20,96,96,96,96,96
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 60) (nb_) = 1; \
   else if ((n_) < 168) (nb_) = 12; \
   else if ((n_) < 360) (nb_) = 20; \
   else (nb_) = 96; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
