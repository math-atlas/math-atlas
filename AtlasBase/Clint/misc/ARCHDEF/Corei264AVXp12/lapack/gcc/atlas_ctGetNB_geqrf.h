#ifndef ATL_ctGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,87,149,211,273,521,645,769,1018,2012,4000
 * N : 25,87,149,211,273,521,645,769,1018,2012,4000
 * NB : 8,20,40,60,60,68,88,88,96,96,96
 */
#define ATL_ctGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 56) (nb_) = 8; \
   else if ((n_) < 118) (nb_) = 20; \
   else if ((n_) < 180) (nb_) = 40; \
   else if ((n_) < 397) (nb_) = 60; \
   else if ((n_) < 583) (nb_) = 68; \
   else if ((n_) < 893) (nb_) = 88; \
   else (nb_) = 96; \
}


#endif    /* end ifndef ATL_ctGetNB_geqrf */
