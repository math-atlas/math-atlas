#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,160,240,320,400,480,560,640,880,1840,3760
 * N : 25,160,240,320,400,480,560,640,880,1840,3760
 * NB : 4,12,12,24,24,40,80,80,80,80,160
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 92) (nb_) = 4; \
   else if ((n_) < 280) (nb_) = 12; \
   else if ((n_) < 440) (nb_) = 24; \
   else if ((n_) < 520) (nb_) = 40; \
   else if ((n_) < 2800) (nb_) = 80; \
   else (nb_) = 160; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
