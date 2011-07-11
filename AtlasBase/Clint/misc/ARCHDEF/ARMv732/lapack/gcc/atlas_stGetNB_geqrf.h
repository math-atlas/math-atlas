#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,160,400,800,960,1200,1600
 * N : 25,160,400,800,960,1200,1600
 * NB : 2,20,20,32,80,80,80
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 92) (nb_) = 2; \
   else if ((n_) < 600) (nb_) = 20; \
   else if ((n_) < 880) (nb_) = 32; \
   else (nb_) = 80; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
