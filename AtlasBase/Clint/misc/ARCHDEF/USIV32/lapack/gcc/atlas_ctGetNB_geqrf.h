#ifndef ATL_ctGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,160,240,320,640,960,1360,2720
 * N : 25,160,240,320,640,960,1360,2720
 * NB : 4,12,16,32,32,80,80,160
 */
#define ATL_ctGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 92) (nb_) = 4; \
   else if ((n_) < 200) (nb_) = 12; \
   else if ((n_) < 280) (nb_) = 16; \
   else if ((n_) < 800) (nb_) = 32; \
   else if ((n_) < 2040) (nb_) = 80; \
   else (nb_) = 160; \
}


#endif    /* end ifndef ATL_ctGetNB_geqrf */
