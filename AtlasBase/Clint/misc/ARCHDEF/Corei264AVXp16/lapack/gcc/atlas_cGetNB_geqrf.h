#ifndef ATL_cGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,122,220,318,416,465,514,563,612,710,808,1200,1592,2376,3160
 * N : 25,122,220,318,416,465,514,563,612,710,808,1200,1592,2376,3160
 * NB : 1,1,27,27,35,35,36,36,42,66,83,95,99,96,163
 */
#define ATL_cGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 171) (nb_) = 1; \
   else if ((n_) < 367) (nb_) = 27; \
   else if ((n_) < 489) (nb_) = 35; \
   else if ((n_) < 587) (nb_) = 36; \
   else if ((n_) < 661) (nb_) = 42; \
   else if ((n_) < 759) (nb_) = 66; \
   else if ((n_) < 1004) (nb_) = 83; \
   else if ((n_) < 1396) (nb_) = 95; \
   else if ((n_) < 1984) (nb_) = 99; \
   else if ((n_) < 2768) (nb_) = 96; \
   else (nb_) = 163; \
}


#endif    /* end ifndef ATL_cGetNB_geqrf */
