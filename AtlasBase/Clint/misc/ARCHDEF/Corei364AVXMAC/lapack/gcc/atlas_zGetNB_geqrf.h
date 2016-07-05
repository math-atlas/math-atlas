#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,144,288,576,864,912,1008,1104,1152,1200,1776,2400
 * N : 25,144,288,576,864,912,1008,1104,1152,1200,1776,2400
 * NB : 4,48,48,48,48,48,56,56,48,144,144,288
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 84) (nb_) = 4; \
   else if ((n_) < 960) (nb_) = 48; \
   else if ((n_) < 1128) (nb_) = 56; \
   else if ((n_) < 1176) (nb_) = 48; \
   else if ((n_) < 2088) (nb_) = 144; \
   else (nb_) = 288; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
