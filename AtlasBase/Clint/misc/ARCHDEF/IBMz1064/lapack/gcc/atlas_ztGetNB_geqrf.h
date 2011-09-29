#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,160,320,640,800,960,1120,1200,1360,2800
 * N : 25,160,320,640,800,960,1120,1200,1360,2800
 * NB : 5,10,20,16,20,20,20,80,80,80
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 92) (nb_) = 5; \
   else if ((n_) < 240) (nb_) = 10; \
   else if ((n_) < 480) (nb_) = 20; \
   else if ((n_) < 720) (nb_) = 16; \
   else if ((n_) < 1160) (nb_) = 20; \
   else (nb_) = 80; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
