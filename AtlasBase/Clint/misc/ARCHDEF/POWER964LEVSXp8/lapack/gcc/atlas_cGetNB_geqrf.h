#ifndef ATL_cGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,76,127,178,230,281,333,384,436,642,848,1260,1672,2496,3320
 * N : 25,76,127,178,230,281,333,384,436,642,848,1260,1672,2496,3320
 * NB : 1,7,11,12,35,59,75,75,83,78,99,96,131,120,195
 */
#define ATL_cGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 50) (nb_) = 1; \
   else if ((n_) < 101) (nb_) = 7; \
   else if ((n_) < 152) (nb_) = 11; \
   else if ((n_) < 204) (nb_) = 12; \
   else if ((n_) < 255) (nb_) = 35; \
   else if ((n_) < 307) (nb_) = 59; \
   else if ((n_) < 410) (nb_) = 75; \
   else if ((n_) < 539) (nb_) = 83; \
   else if ((n_) < 745) (nb_) = 78; \
   else if ((n_) < 1054) (nb_) = 99; \
   else if ((n_) < 1466) (nb_) = 96; \
   else if ((n_) < 2084) (nb_) = 131; \
   else if ((n_) < 2908) (nb_) = 120; \
   else (nb_) = 195; \
}


#endif    /* end ifndef ATL_cGetNB_geqrf */
