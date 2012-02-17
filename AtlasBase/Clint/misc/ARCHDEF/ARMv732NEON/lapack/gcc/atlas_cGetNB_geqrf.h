#ifndef ATL_cGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,160,400,480,560,640,880
 * N : 25,160,400,480,560,640,880
 * NB : 4,16,20,32,40,80,80
 */
#define ATL_cGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 92) (nb_) = 4; \
   else if ((n_) < 280) (nb_) = 16; \
   else if ((n_) < 440) (nb_) = 20; \
   else if ((n_) < 520) (nb_) = 32; \
   else if ((n_) < 600) (nb_) = 40; \
   else (nb_) = 80; \
}


#endif    /* end ifndef ATL_cGetNB_geqrf */
