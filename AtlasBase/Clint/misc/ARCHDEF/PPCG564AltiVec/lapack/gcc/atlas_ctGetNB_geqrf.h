#ifndef ATL_ctGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,240,320,400,480,1040,2080
 * N : 25,240,320,400,480,1040,2080
 * NB : 4,20,32,32,80,80,80
 */
#define ATL_ctGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 132) (nb_) = 4; \
   else if ((n_) < 280) (nb_) = 20; \
   else if ((n_) < 440) (nb_) = 32; \
   else (nb_) = 80; \
}


#endif    /* end ifndef ATL_ctGetNB_geqrf */
