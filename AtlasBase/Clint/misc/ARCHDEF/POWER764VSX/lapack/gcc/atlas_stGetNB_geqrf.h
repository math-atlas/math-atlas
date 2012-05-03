#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,240,320,400,560,640,720,880,1200,2480,4960,7440,8720,10000
 * N : 25,240,320,400,560,640,720,880,1200,2480,4960,7440,8720,10000
 * NB : 4,20,28,104,80,160,160,160,160,160,240,240,240,400
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 132) (nb_) = 4; \
   else if ((n_) < 280) (nb_) = 20; \
   else if ((n_) < 360) (nb_) = 28; \
   else if ((n_) < 480) (nb_) = 104; \
   else if ((n_) < 600) (nb_) = 80; \
   else if ((n_) < 3720) (nb_) = 160; \
   else if ((n_) < 9360) (nb_) = 240; \
   else (nb_) = 400; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
