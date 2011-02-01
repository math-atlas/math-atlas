#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,240,320,400,560,640,720,880,960,1040,1120,1200,2480,4960,10000
 * N : 25,240,320,400,560,640,720,880,960,1040,1120,1200,2480,4960,10000
 * NB : 4,16,16,20,20,32,32,28,32,32,40,80,80,80,160
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 132) (nb_) = 4; \
   else if ((n_) < 360) (nb_) = 16; \
   else if ((n_) < 600) (nb_) = 20; \
   else if ((n_) < 800) (nb_) = 32; \
   else if ((n_) < 920) (nb_) = 28; \
   else if ((n_) < 1080) (nb_) = 32; \
   else if ((n_) < 1160) (nb_) = 40; \
   else if ((n_) < 7480) (nb_) = 80; \
   else (nb_) = 160; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
