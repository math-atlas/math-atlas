#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,120,200,240,280,560,1120,2280
 * N : 25,120,200,240,280,560,1120,2280
 * NB : 2,20,20,20,40,40,40,40
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 72) (nb_) = 2; \
   else if ((n_) < 260) (nb_) = 20; \
   else (nb_) = 40; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
