#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='LOWER'
 * M : 25,160,240,320,400,800,1600,2000,2160,2240,2400,3280,6560
 * N : 25,160,240,320,400,800,1600,2000,2160,2240,2400,3280,6560
 * NB : 4,12,16,80,80,80,80,80,80,80,160,180,160
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 92) (nb_) = 4; \
   else if ((n_) < 200) (nb_) = 12; \
   else if ((n_) < 280) (nb_) = 16; \
   else if ((n_) < 2320) (nb_) = 80; \
   else if ((n_) < 2840) (nb_) = 160; \
   else if ((n_) < 4920) (nb_) = 180; \
   else (nb_) = 160; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
