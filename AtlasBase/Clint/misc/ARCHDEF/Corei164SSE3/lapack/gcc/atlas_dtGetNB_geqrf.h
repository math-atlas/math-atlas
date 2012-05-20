#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,180,360,540,600,720,1500,3000,4500,6060
 * N : 25,180,360,540,600,720,1500,3000,4500,6060
 * NB : 2,60,60,60,120,120,120,120,120,144
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 102) (nb_) = 2; \
   else if ((n_) < 570) (nb_) = 60; \
   else if ((n_) < 5280) (nb_) = 120; \
   else (nb_) = 144; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
