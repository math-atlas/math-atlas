#ifndef ATL_cGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,240,320,480,1040,2080
 * N : 25,240,320,480,1040,2080
 * NB : 12,12,80,80,80,80
 */
#define ATL_cGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 280) (nb_) = 12; \
   else (nb_) = 80; \
}


#endif    /* end ifndef ATL_cGetNB_geqrf */
