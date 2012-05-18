#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,160,240,320,400,880,1760,3600,7200
 * N : 25,160,240,320,400,880,1760,3600,7200
 * NB : 4,12,16,80,80,80,80,80,160
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 92) (nb_) = 4; \
   else if ((n_) < 200) (nb_) = 12; \
   else if ((n_) < 280) (nb_) = 16; \
   else if ((n_) < 5400) (nb_) = 80; \
   else (nb_) = 160; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
