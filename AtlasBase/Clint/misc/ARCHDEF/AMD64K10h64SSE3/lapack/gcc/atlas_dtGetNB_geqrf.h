#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,120,160,200,280,560,1120,2280,4560
 * N : 25,120,160,200,280,560,1120,2280,4560
 * NB : 4,8,40,40,40,40,80,80,80
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 72) (nb_) = 4; \
   else if ((n_) < 140) (nb_) = 8; \
   else if ((n_) < 840) (nb_) = 40; \
   else (nb_) = 80; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
