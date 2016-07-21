#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,80,120,160,200,400,440,480,600,680,800,1200,1640,3280
 * N : 25,80,120,160,200,400,440,480,600,680,800,1200,1640,3280
 * NB : 2,8,8,16,16,20,40,40,40,40,80,80,120,120
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 52) (nb_) = 2; \
   else if ((n_) < 140) (nb_) = 8; \
   else if ((n_) < 300) (nb_) = 16; \
   else if ((n_) < 420) (nb_) = 20; \
   else if ((n_) < 740) (nb_) = 40; \
   else if ((n_) < 1420) (nb_) = 80; \
   else (nb_) = 120; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
