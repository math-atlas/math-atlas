#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,160,240,400,480,560,640,880,1760,3520
 * N : 25,160,240,400,480,560,640,880,1760,3520
 * NB : 4,8,8,16,32,80,80,80,80,80
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 92) (nb_) = 4; \
   else if ((n_) < 320) (nb_) = 8; \
   else if ((n_) < 440) (nb_) = 16; \
   else if ((n_) < 520) (nb_) = 32; \
   else (nb_) = 80; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
