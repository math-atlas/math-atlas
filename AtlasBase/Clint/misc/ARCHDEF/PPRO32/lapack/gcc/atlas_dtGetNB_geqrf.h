#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,80,120,160,200,400,600,800
 * N : 25,80,120,160,200,400,600,800
 * NB : 2,8,8,40,40,40,80,80
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 52) (nb_) = 2; \
   else if ((n_) < 140) (nb_) = 8; \
   else if ((n_) < 500) (nb_) = 40; \
   else (nb_) = 80; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
