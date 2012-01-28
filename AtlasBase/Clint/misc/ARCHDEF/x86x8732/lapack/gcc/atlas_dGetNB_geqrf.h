#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,150,300,600,900,1200,2430
 * N : 25,150,300,600,900,1200,2430
 * NB : 5,30,30,30,30,60,60
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 87) (nb_) = 5; \
   else if ((n_) < 1050) (nb_) = 30; \
   else (nb_) = 60; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
