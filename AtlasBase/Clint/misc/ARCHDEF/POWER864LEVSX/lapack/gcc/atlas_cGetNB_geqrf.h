#ifndef ATL_cGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,240,560,1120,2320
 * N : 25,240,560,1120,2320
 * NB : 12,80,80,80,80
 */
#define ATL_cGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 132) (nb_) = 12; \
   else (nb_) = 80; \
}


#endif    /* end ifndef ATL_cGetNB_geqrf */
