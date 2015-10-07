#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,88,132,176,264,308,352,528,704,1452
 * N : 25,88,132,176,264,308,352,528,704,1452
 * NB : 2,8,12,16,16,44,44,44,88,88
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 56) (nb_) = 2; \
   else if ((n_) < 110) (nb_) = 8; \
   else if ((n_) < 154) (nb_) = 12; \
   else if ((n_) < 286) (nb_) = 16; \
   else if ((n_) < 616) (nb_) = 44; \
   else (nb_) = 88; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
