#ifndef ATL_cGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,320,640,800,960,1280
 * N : 25,320,640,800,960,1280
 * NB : 12,12,20,80,80,80
 */
#define ATL_cGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 480) (nb_) = 12; \
   else if ((n_) < 720) (nb_) = 20; \
   else (nb_) = 80; \
}


#endif    /* end ifndef ATL_cGetNB_geqrf */
