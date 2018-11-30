#ifndef ATL_cGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,67,110,152,195,237,280,322,365,535,620,706,876,1047,1217,1388,1729,2070,2752,5480
 * N : 25,67,110,152,195,237,280,322,365,535,620,706,876,1047,1217,1388,1729,2070,2752,5480
 * NB : 1,5,23,18,35,30,47,48,51,48,48,83,91,95,97,99,99,352,352,352
 */
#define ATL_cGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 46) (nb_) = 1; \
   else if ((n_) < 88) (nb_) = 5; \
   else if ((n_) < 131) (nb_) = 23; \
   else if ((n_) < 173) (nb_) = 18; \
   else if ((n_) < 216) (nb_) = 35; \
   else if ((n_) < 258) (nb_) = 30; \
   else if ((n_) < 301) (nb_) = 47; \
   else if ((n_) < 343) (nb_) = 48; \
   else if ((n_) < 450) (nb_) = 51; \
   else if ((n_) < 663) (nb_) = 48; \
   else if ((n_) < 791) (nb_) = 83; \
   else if ((n_) < 961) (nb_) = 91; \
   else if ((n_) < 1132) (nb_) = 95; \
   else if ((n_) < 1302) (nb_) = 97; \
   else if ((n_) < 1899) (nb_) = 99; \
   else (nb_) = 352; \
}


#endif    /* end ifndef ATL_cGetNB_geqrf */
