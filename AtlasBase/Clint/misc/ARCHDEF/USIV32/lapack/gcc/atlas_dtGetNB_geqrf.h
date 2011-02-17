#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,504,672,1008,1512,2184,4368
 * N : 25,504,672,1008,1512,2184,4368
 * NB : 2,20,32,56,56,168,336
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 264) (nb_) = 2; \
   else if ((n_) < 588) (nb_) = 20; \
   else if ((n_) < 840) (nb_) = 32; \
   else if ((n_) < 1848) (nb_) = 56; \
   else if ((n_) < 3276) (nb_) = 168; \
   else (nb_) = 336; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
