#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,165,305,340,375,410,445,515,550,585,865,1146,1707,2268,2829,3390,3951,4512,6756,9000
 * N : 25,165,305,340,375,410,445,515,550,585,865,1146,1707,2268,2829,3390,3951,4512,6756,9000
 * NB : 1,1,11,43,59,67,75,70,74,99,107,115,115,131,143,147,147,186,420,420
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 235) (nb_) = 1; \
   else if ((n_) < 322) (nb_) = 11; \
   else if ((n_) < 357) (nb_) = 43; \
   else if ((n_) < 392) (nb_) = 59; \
   else if ((n_) < 427) (nb_) = 67; \
   else if ((n_) < 480) (nb_) = 75; \
   else if ((n_) < 532) (nb_) = 70; \
   else if ((n_) < 567) (nb_) = 74; \
   else if ((n_) < 725) (nb_) = 99; \
   else if ((n_) < 1005) (nb_) = 107; \
   else if ((n_) < 1987) (nb_) = 115; \
   else if ((n_) < 2548) (nb_) = 131; \
   else if ((n_) < 3109) (nb_) = 143; \
   else if ((n_) < 4231) (nb_) = 147; \
   else if ((n_) < 5634) (nb_) = 186; \
   else (nb_) = 420; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
