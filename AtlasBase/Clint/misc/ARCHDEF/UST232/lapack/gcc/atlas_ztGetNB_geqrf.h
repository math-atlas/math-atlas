#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,224,272,304,336,448,896,1328,1776,3552
 * N : 25,224,272,304,336,448,896,1328,1776,3552
 * NB : 9,32,32,32,64,64,64,64,80,96
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 124) (nb_) = 9; \
   else if ((n_) < 320) (nb_) = 32; \
   else if ((n_) < 1552) (nb_) = 64; \
   else if ((n_) < 2664) (nb_) = 80; \
   else (nb_) = 96; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
