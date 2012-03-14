#ifndef ATL_ctGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,228,456,912,1900,3800
 * N : 25,228,456,912,1900,3800
 * NB : 5,76,76,76,76,152
 */
#define ATL_ctGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 126) (nb_) = 5; \
   else if ((n_) < 2850) (nb_) = 76; \
   else (nb_) = 152; \
}


#endif    /* end ifndef ATL_ctGetNB_geqrf */
