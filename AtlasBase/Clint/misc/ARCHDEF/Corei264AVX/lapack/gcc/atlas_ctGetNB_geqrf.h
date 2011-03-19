#ifndef ATL_ctGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,240,320,400,480,960,1920,3920
 * N : 25,240,320,400,480,960,1920,3920
 * NB : 7,80,80,80,84,80,80,80
 */
#define ATL_ctGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 132) (nb_) = 7; \
   else if ((n_) < 440) (nb_) = 80; \
   else if ((n_) < 720) (nb_) = 84; \
   else (nb_) = 80; \
}


#endif    /* end ifndef ATL_ctGetNB_geqrf */
