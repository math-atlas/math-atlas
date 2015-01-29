#ifndef ATL_ctGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,87,149,211,273,335,397,459,521,769,1018,2012,3006,4000
 * N : 25,87,149,211,273,335,397,459,521,769,1018,2012,3006,4000
 * NB : 24,24,28,32,32,76,76,76,80,88,88,124,124,144
 */
#define ATL_ctGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 118) (nb_) = 24; \
   else if ((n_) < 180) (nb_) = 28; \
   else if ((n_) < 304) (nb_) = 32; \
   else if ((n_) < 490) (nb_) = 76; \
   else if ((n_) < 645) (nb_) = 80; \
   else if ((n_) < 1515) (nb_) = 88; \
   else if ((n_) < 3503) (nb_) = 124; \
   else (nb_) = 144; \
}


#endif    /* end ifndef ATL_ctGetNB_geqrf */
