#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,87,149,273,335,397,459,521,1018,1515,2012,4000
 * N : 25,87,149,273,335,397,459,521,1018,1515,2012,4000
 * NB : 8,8,12,12,56,60,60,80,92,120,120,132
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 118) (nb_) = 8; \
   else if ((n_) < 304) (nb_) = 12; \
   else if ((n_) < 366) (nb_) = 56; \
   else if ((n_) < 490) (nb_) = 60; \
   else if ((n_) < 769) (nb_) = 80; \
   else if ((n_) < 1266) (nb_) = 92; \
   else if ((n_) < 3006) (nb_) = 120; \
   else (nb_) = 132; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
