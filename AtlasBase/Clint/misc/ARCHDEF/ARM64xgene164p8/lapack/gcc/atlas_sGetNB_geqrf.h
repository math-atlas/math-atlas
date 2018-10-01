#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,115,205,250,295,340,386,431,476,521,567,657,748,1110,1472,2196,2920
 * N : 25,115,205,250,295,340,386,431,476,521,567,657,748,1110,1472,2196,2920
 * NB : 1,1,4,12,16,16,19,19,20,20,23,23,35,35,43,47,67
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 160) (nb_) = 1; \
   else if ((n_) < 227) (nb_) = 4; \
   else if ((n_) < 272) (nb_) = 12; \
   else if ((n_) < 363) (nb_) = 16; \
   else if ((n_) < 453) (nb_) = 19; \
   else if ((n_) < 544) (nb_) = 20; \
   else if ((n_) < 702) (nb_) = 23; \
   else if ((n_) < 1291) (nb_) = 35; \
   else if ((n_) < 1834) (nb_) = 43; \
   else if ((n_) < 2558) (nb_) = 47; \
   else (nb_) = 67; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
