#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,80,120,160,320,640,800,960,1280,2600
 * N : 25,80,120,160,320,640,800,960,1280,2600
 * NB : 4,4,10,40,40,20,40,40,80,120
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 100) (nb_) = 4; \
   else if ((n_) < 140) (nb_) = 10; \
   else if ((n_) < 480) (nb_) = 40; \
   else if ((n_) < 720) (nb_) = 20; \
   else if ((n_) < 1120) (nb_) = 40; \
   else if ((n_) < 1940) (nb_) = 80; \
   else (nb_) = 120; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
