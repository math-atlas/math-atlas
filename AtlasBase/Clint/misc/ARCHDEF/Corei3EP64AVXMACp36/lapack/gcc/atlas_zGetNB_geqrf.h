#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,72,120,167,215,310,358,406,597,788,1170,1552,2316,3080
 * N : 25,72,120,167,215,310,358,406,597,788,1170,1552,2316,3080
 * NB : 1,1,15,23,27,24,36,51,63,83,72,131,147,163
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 96) (nb_) = 1; \
   else if ((n_) < 143) (nb_) = 15; \
   else if ((n_) < 191) (nb_) = 23; \
   else if ((n_) < 262) (nb_) = 27; \
   else if ((n_) < 334) (nb_) = 24; \
   else if ((n_) < 382) (nb_) = 36; \
   else if ((n_) < 501) (nb_) = 51; \
   else if ((n_) < 692) (nb_) = 63; \
   else if ((n_) < 979) (nb_) = 83; \
   else if ((n_) < 1361) (nb_) = 72; \
   else if ((n_) < 1934) (nb_) = 131; \
   else if ((n_) < 2698) (nb_) = 147; \
   else (nb_) = 163; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
