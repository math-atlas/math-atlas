#ifndef ATL_ctGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,240,320,400,480,720,880,960,1040,2080,3120,3600,4160,8320
 * N : 25,240,320,400,480,720,880,960,1040,2080,3120,3600,4160,8320
 * NB : 4,24,24,24,28,24,20,80,80,80,80,160,160,320
 */
#define ATL_ctGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 132) (nb_) = 4; \
   else if ((n_) < 440) (nb_) = 24; \
   else if ((n_) < 600) (nb_) = 28; \
   else if ((n_) < 800) (nb_) = 24; \
   else if ((n_) < 920) (nb_) = 20; \
   else if ((n_) < 3360) (nb_) = 80; \
   else if ((n_) < 6240) (nb_) = 160; \
   else (nb_) = 320; \
}


#endif    /* end ifndef ATL_ctGetNB_geqrf */
