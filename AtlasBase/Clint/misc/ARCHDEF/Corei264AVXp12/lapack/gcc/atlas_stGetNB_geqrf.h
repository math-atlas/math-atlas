#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,273,335,397,521,1018,1266,1328,1390,1515,1763,2012,4000
 * N : 25,273,335,397,521,1018,1266,1328,1390,1515,1763,2012,4000
 * NB : 12,20,48,52,52,68,68,68,76,80,84,108,132
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 149) (nb_) = 12; \
   else if ((n_) < 304) (nb_) = 20; \
   else if ((n_) < 366) (nb_) = 48; \
   else if ((n_) < 769) (nb_) = 52; \
   else if ((n_) < 1359) (nb_) = 68; \
   else if ((n_) < 1452) (nb_) = 76; \
   else if ((n_) < 1639) (nb_) = 80; \
   else if ((n_) < 1887) (nb_) = 84; \
   else if ((n_) < 3006) (nb_) = 108; \
   else (nb_) = 132; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
