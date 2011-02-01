#ifndef ATL_ctGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='LOWER'
 * M : 25,168,420,672,756,840,924,1344,1848,3780
 * N : 25,168,420,672,756,840,924,1344,1848,3780
 * NB : 9,16,16,16,16,84,84,168,168,168
 */
#define ATL_ctGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 96) (nb_) = 9; \
   else if ((n_) < 798) (nb_) = 16; \
   else if ((n_) < 1134) (nb_) = 84; \
   else (nb_) = 168; \
}


#endif    /* end ifndef ATL_ctGetNB_geqrf */
