#ifndef ATL_ctGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,240,480,720,880,1040,1520,1760,2080
 * N : 25,240,480,720,880,1040,1520,1760,2080
 * NB : 5,40,20,20,80,80,80,80,160
 */
#define ATL_ctGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 132) (nb_) = 5; \
   else if ((n_) < 360) (nb_) = 40; \
   else if ((n_) < 800) (nb_) = 20; \
   else if ((n_) < 1920) (nb_) = 80; \
   else (nb_) = 160; \
}


#endif    /* end ifndef ATL_ctGetNB_geqrf */
