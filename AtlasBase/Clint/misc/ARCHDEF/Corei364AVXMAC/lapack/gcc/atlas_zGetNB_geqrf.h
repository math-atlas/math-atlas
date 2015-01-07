#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,97,170,243,316,462,608,900,1192,2360
 * N : 25,97,170,243,316,462,608,900,1192,2360
 * NB : 4,12,12,24,40,48,48,60,88,120
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 61) (nb_) = 4; \
   else if ((n_) < 206) (nb_) = 12; \
   else if ((n_) < 279) (nb_) = 24; \
   else if ((n_) < 389) (nb_) = 40; \
   else if ((n_) < 754) (nb_) = 48; \
   else if ((n_) < 1046) (nb_) = 60; \
   else if ((n_) < 1776) (nb_) = 88; \
   else (nb_) = 120; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
