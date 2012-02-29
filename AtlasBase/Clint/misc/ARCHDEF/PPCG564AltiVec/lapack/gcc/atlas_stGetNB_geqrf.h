#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,160,240,320,400,880,1120,1200,1280,1360,1440,1520,1600,1840,3760
 * N : 25,160,240,320,400,880,1120,1200,1280,1360,1440,1520,1600,1840,3760
 * NB : 4,12,16,80,80,80,64,144,80,192,144,80,240,240,240
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 92) (nb_) = 4; \
   else if ((n_) < 200) (nb_) = 12; \
   else if ((n_) < 280) (nb_) = 16; \
   else if ((n_) < 1000) (nb_) = 80; \
   else if ((n_) < 1160) (nb_) = 64; \
   else if ((n_) < 1240) (nb_) = 144; \
   else if ((n_) < 1320) (nb_) = 80; \
   else if ((n_) < 1400) (nb_) = 192; \
   else if ((n_) < 1480) (nb_) = 144; \
   else if ((n_) < 1560) (nb_) = 80; \
   else (nb_) = 240; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
