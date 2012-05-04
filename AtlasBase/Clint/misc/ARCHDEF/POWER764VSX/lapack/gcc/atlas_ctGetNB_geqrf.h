#ifndef ATL_ctGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,240,480,720,880,1040,2160,4320,4800,5360,5600,5920,6480,8640
 * N : 25,240,480,720,880,1040,2160,4320,4800,5360,5600,5920,6480,8640
 * NB : 5,80,80,80,80,160,160,160,320,320,320,400,400,400
 */
#define ATL_ctGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 132) (nb_) = 5; \
   else if ((n_) < 960) (nb_) = 80; \
   else if ((n_) < 4560) (nb_) = 160; \
   else if ((n_) < 5760) (nb_) = 320; \
   else (nb_) = 400; \
}


#endif    /* end ifndef ATL_ctGetNB_geqrf */
