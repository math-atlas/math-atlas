#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,86,148,271,332,394,518,1012,2000
 * N : 25,86,148,271,332,394,518,1012,2000
 * NB : 6,12,24,24,36,60,60,60,132
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 55) (nb_) = 6; \
   else if ((n_) < 117) (nb_) = 12; \
   else if ((n_) < 301) (nb_) = 24; \
   else if ((n_) < 363) (nb_) = 36; \
   else if ((n_) < 1506) (nb_) = 60; \
   else (nb_) = 132; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
