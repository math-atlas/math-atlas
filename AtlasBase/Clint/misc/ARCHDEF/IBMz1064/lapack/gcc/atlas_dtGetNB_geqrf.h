#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,240,560,1120,1200,1280,1360,1680,2240,4480
 * N : 25,240,560,1120,1200,1280,1360,1680,2240,4480
 * NB : 5,16,16,20,30,30,80,80,80,160
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 132) (nb_) = 5; \
   else if ((n_) < 840) (nb_) = 16; \
   else if ((n_) < 1160) (nb_) = 20; \
   else if ((n_) < 1320) (nb_) = 30; \
   else if ((n_) < 3360) (nb_) = 80; \
   else (nb_) = 160; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
