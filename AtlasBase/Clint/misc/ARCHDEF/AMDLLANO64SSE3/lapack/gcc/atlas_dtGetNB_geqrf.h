#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,93,162,300,576,852,990,1059,1128,2232,3336,4440
 * N : 25,93,162,300,576,852,990,1059,1128,2232,3336,4440
 * NB : 8,12,12,12,12,12,12,14,14,14,14,40
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 59) (nb_) = 8; \
   else if ((n_) < 1024) (nb_) = 12; \
   else if ((n_) < 3888) (nb_) = 14; \
   else (nb_) = 40; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
