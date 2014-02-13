#ifndef ATL_ctGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,240,560,1120,2240,4560
 * N : 25,240,560,1120,2240,4560
 * NB : 9,80,80,80,80,80
 */
#define ATL_ctGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 132) (nb_) = 9; \
   else (nb_) = 80; \
}


#endif    /* end ifndef ATL_ctGetNB_geqrf */
