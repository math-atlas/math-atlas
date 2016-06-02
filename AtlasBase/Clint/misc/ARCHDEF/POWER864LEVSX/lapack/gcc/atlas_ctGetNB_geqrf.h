#ifndef ATL_ctGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,160,240,400,880,1840,3760
 * N : 25,160,240,400,880,1840,3760
 * NB : 5,16,80,80,80,80,80
 */
#define ATL_ctGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 92) (nb_) = 5; \
   else if ((n_) < 200) (nb_) = 16; \
   else (nb_) = 80; \
}


#endif    /* end ifndef ATL_ctGetNB_geqrf */
