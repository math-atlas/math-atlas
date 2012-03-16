#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,160,240,320,400,880,1360,1440,1520,1600,1840,3760
 * N : 25,160,240,320,400,880,1360,1440,1520,1600,1840,3760
 * NB : 4,16,20,20,32,80,80,80,240,240,240,240
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 92) (nb_) = 4; \
   else if ((n_) < 200) (nb_) = 16; \
   else if ((n_) < 360) (nb_) = 20; \
   else if ((n_) < 640) (nb_) = 32; \
   else if ((n_) < 1480) (nb_) = 80; \
   else (nb_) = 240; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
