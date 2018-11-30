#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,58,92,126,160,193,227,261,295,430,566,837,1108,1379,1650,1921,2192,3276,4360
 * N : 25,58,92,126,160,193,227,261,295,430,566,837,1108,1379,1650,1921,2192,3276,4360
 * NB : 1,1,23,24,27,28,39,40,43,47,51,51,86,102,134,134,240,288,432
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 75) (nb_) = 1; \
   else if ((n_) < 109) (nb_) = 23; \
   else if ((n_) < 143) (nb_) = 24; \
   else if ((n_) < 176) (nb_) = 27; \
   else if ((n_) < 210) (nb_) = 28; \
   else if ((n_) < 244) (nb_) = 39; \
   else if ((n_) < 278) (nb_) = 40; \
   else if ((n_) < 362) (nb_) = 43; \
   else if ((n_) < 498) (nb_) = 47; \
   else if ((n_) < 972) (nb_) = 51; \
   else if ((n_) < 1243) (nb_) = 86; \
   else if ((n_) < 1514) (nb_) = 102; \
   else if ((n_) < 2056) (nb_) = 134; \
   else if ((n_) < 2734) (nb_) = 240; \
   else if ((n_) < 3818) (nb_) = 288; \
   else (nb_) = 432; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
