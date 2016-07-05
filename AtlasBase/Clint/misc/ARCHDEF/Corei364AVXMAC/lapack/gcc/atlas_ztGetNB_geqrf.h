#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,144,288,576,864,1200,1776,2064,2400,4848
 * N : 25,144,288,576,864,1200,1776,2064,2400,4848
 * NB : 3,48,48,48,48,56,56,56,144,144
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 84) (nb_) = 3; \
   else if ((n_) < 1032) (nb_) = 48; \
   else if ((n_) < 2232) (nb_) = 56; \
   else (nb_) = 144; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
