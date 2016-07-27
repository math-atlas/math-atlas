#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,240,560,1200,2400,3600,4160,4480,4560,4640,4720,4800
 * N : 25,240,560,1200,2400,3600,4160,4480,4560,4640,4720,4800
 * NB : 4,80,80,80,80,80,80,80,240,320,560,640
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 132) (nb_) = 4; \
   else if ((n_) < 4520) (nb_) = 80; \
   else if ((n_) < 4600) (nb_) = 240; \
   else if ((n_) < 4680) (nb_) = 320; \
   else if ((n_) < 4760) (nb_) = 560; \
   else (nb_) = 640; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
