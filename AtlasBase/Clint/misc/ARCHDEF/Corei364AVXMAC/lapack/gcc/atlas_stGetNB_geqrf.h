#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,160,240,320,400,880,1840,1920,2080,2160,2240,2320,2800,3760,7520
 * N : 25,160,240,320,400,880,1840,1920,2080,2160,2240,2320,2800,3760,7520
 * NB : 2,16,16,80,80,80,80,160,160,160,160,400,400,400,400
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 92) (nb_) = 2; \
   else if ((n_) < 280) (nb_) = 16; \
   else if ((n_) < 1880) (nb_) = 80; \
   else if ((n_) < 2280) (nb_) = 160; \
   else (nb_) = 400; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
