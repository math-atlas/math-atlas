#ifndef ATL_cGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,132,240,294,348,402,456,672,888,1320,1752,2616,3480
 * N : 25,132,240,294,348,402,456,672,888,1320,1752,2616,3480
 * NB : 1,1,4,24,28,28,35,55,67,107,131,120,195
 */
#define ATL_cGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 186) (nb_) = 1; \
   else if ((n_) < 267) (nb_) = 4; \
   else if ((n_) < 321) (nb_) = 24; \
   else if ((n_) < 429) (nb_) = 28; \
   else if ((n_) < 564) (nb_) = 35; \
   else if ((n_) < 780) (nb_) = 55; \
   else if ((n_) < 1104) (nb_) = 67; \
   else if ((n_) < 1536) (nb_) = 107; \
   else if ((n_) < 2184) (nb_) = 131; \
   else if ((n_) < 3048) (nb_) = 120; \
   else (nb_) = 195; \
}


#endif    /* end ifndef ATL_cGetNB_geqrf */
