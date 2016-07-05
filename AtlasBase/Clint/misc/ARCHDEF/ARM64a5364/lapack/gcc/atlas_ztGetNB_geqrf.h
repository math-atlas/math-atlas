#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,96,192,432,480,528,624,864,1776
 * N : 25,96,192,432,480,528,624,864,1776
 * NB : 4,12,24,16,48,48,48,48,48
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 60) (nb_) = 4; \
   else if ((n_) < 144) (nb_) = 12; \
   else if ((n_) < 312) (nb_) = 24; \
   else if ((n_) < 456) (nb_) = 16; \
   else (nb_) = 48; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
