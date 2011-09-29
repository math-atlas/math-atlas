#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,160,320,640,1360
 * N : 25,160,320,640,1360
 * NB : 5,10,20,30,40
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 92) (nb_) = 5; \
   else if ((n_) < 240) (nb_) = 10; \
   else if ((n_) < 480) (nb_) = 20; \
   else if ((n_) < 1000) (nb_) = 30; \
   else (nb_) = 40; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
