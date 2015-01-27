#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,87,149,273,335,397,521,769,893,1018,1515,2012,3006,4000
 * N : 25,87,149,273,335,397,521,769,893,1018,1515,2012,3006,4000
 * NB : 6,8,8,8,8,56,60,60,60,92,124,124,124,136
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 56) (nb_) = 6; \
   else if ((n_) < 366) (nb_) = 8; \
   else if ((n_) < 459) (nb_) = 56; \
   else if ((n_) < 955) (nb_) = 60; \
   else if ((n_) < 1266) (nb_) = 92; \
   else if ((n_) < 3503) (nb_) = 124; \
   else (nb_) = 136; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
