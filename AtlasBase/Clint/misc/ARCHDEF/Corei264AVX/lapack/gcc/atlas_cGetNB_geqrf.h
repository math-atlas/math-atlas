#ifndef ATL_cGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,168,420,924,1848
 * N : 25,168,420,924,1848
 * NB : 9,84,84,84,84
 */
#define ATL_cGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 96) (nb_) = 9; \
   else (nb_) = 84; \
}


#endif    /* end ifndef ATL_cGetNB_geqrf */
