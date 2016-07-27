#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,160,400,480,560,640,880,1280,1760,3520
 * N : 25,160,400,480,560,640,880,1280,1760,3520
 * NB : 5,20,16,40,80,80,80,80,160,160
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 92) (nb_) = 5; \
   else if ((n_) < 280) (nb_) = 20; \
   else if ((n_) < 440) (nb_) = 16; \
   else if ((n_) < 520) (nb_) = 40; \
   else if ((n_) < 1520) (nb_) = 80; \
   else (nb_) = 160; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
