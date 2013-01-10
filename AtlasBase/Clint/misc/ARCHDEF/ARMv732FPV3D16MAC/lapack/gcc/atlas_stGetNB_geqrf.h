#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,192,256,320,384,448,640,768,896,1344,1600,1728,1856
 * N : 25,192,256,320,384,448,640,768,896,1344,1600,1728,1856
 * NB : 1,12,20,24,24,64,64,64,128,128,96,128,192
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 108) (nb_) = 1; \
   else if ((n_) < 224) (nb_) = 12; \
   else if ((n_) < 288) (nb_) = 20; \
   else if ((n_) < 416) (nb_) = 24; \
   else if ((n_) < 832) (nb_) = 64; \
   else if ((n_) < 1472) (nb_) = 128; \
   else if ((n_) < 1664) (nb_) = 96; \
   else if ((n_) < 1792) (nb_) = 128; \
   else (nb_) = 192; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
