#ifndef ATL_ctGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,240,480,1040,1280,1520,2080
 * N : 25,240,480,1040,1280,1520,2080
 * NB : 4,40,20,24,80,80,80
 */
#define ATL_ctGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 132) (nb_) = 4; \
   else if ((n_) < 360) (nb_) = 40; \
   else if ((n_) < 760) (nb_) = 20; \
   else if ((n_) < 1160) (nb_) = 24; \
   else (nb_) = 80; \
}


#endif    /* end ifndef ATL_ctGetNB_geqrf */
