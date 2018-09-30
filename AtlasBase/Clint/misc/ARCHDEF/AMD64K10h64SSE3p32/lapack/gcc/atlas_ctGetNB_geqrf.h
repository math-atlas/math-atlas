#ifndef ATL_ctGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,202,380,424,469,513,558,602,647,691,736,1092,1448,2160,2872,4296,5720
 * N : 25,202,380,424,469,513,558,602,647,691,736,1092,1448,2160,2872,4296,5720
 * NB : 1,1,11,19,23,31,35,38,47,48,51,48,67,91,99,99,163
 */
#define ATL_ctGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 291) (nb_) = 1; \
   else if ((n_) < 402) (nb_) = 11; \
   else if ((n_) < 446) (nb_) = 19; \
   else if ((n_) < 491) (nb_) = 23; \
   else if ((n_) < 535) (nb_) = 31; \
   else if ((n_) < 580) (nb_) = 35; \
   else if ((n_) < 624) (nb_) = 38; \
   else if ((n_) < 669) (nb_) = 47; \
   else if ((n_) < 713) (nb_) = 48; \
   else if ((n_) < 914) (nb_) = 51; \
   else if ((n_) < 1270) (nb_) = 48; \
   else if ((n_) < 1804) (nb_) = 67; \
   else if ((n_) < 2516) (nb_) = 91; \
   else if ((n_) < 5008) (nb_) = 99; \
   else (nb_) = 163; \
}


#endif    /* end ifndef ATL_ctGetNB_geqrf */
