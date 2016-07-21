#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,192,384,576,640,704,832,1216,1280,1408,1664,3328
 * N : 25,192,384,576,640,704,832,1216,1280,1408,1664,3328
 * NB : 2,8,16,16,64,64,64,64,128,128,128,128
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 108) (nb_) = 2; \
   else if ((n_) < 288) (nb_) = 8; \
   else if ((n_) < 608) (nb_) = 16; \
   else if ((n_) < 1248) (nb_) = 64; \
   else (nb_) = 128; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
