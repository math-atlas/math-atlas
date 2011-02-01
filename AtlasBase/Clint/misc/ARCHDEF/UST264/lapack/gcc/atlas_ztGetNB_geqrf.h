#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,112,128,160,224,448,896,1344,1456,1568,1792,3584
 * N : 25,112,128,160,224,448,896,1344,1456,1568,1792,3584
 * NB : 9,8,16,32,32,32,64,64,64,80,96,96
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 68) (nb_) = 9; \
   else if ((n_) < 120) (nb_) = 8; \
   else if ((n_) < 144) (nb_) = 16; \
   else if ((n_) < 672) (nb_) = 32; \
   else if ((n_) < 1512) (nb_) = 64; \
   else if ((n_) < 1680) (nb_) = 80; \
   else (nb_) = 96; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
