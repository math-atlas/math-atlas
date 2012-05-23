#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,160,240,320,400,880,1360,1440,1600,1840,2800,3760
 * N : 25,160,240,320,400,880,1360,1440,1600,1840,2800,3760
 * NB : 4,8,12,32,80,80,80,160,160,240,560,560
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 92) (nb_) = 4; \
   else if ((n_) < 200) (nb_) = 8; \
   else if ((n_) < 280) (nb_) = 12; \
   else if ((n_) < 360) (nb_) = 32; \
   else if ((n_) < 1400) (nb_) = 80; \
   else if ((n_) < 1720) (nb_) = 160; \
   else if ((n_) < 2320) (nb_) = 240; \
   else (nb_) = 560; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
