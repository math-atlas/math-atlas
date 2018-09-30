#ifndef ATL_cGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,130,235,287,340,393,446,551,657,762,868,1290,1712,2556,3400
 * N : 25,130,235,287,340,393,446,551,657,762,868,1290,1712,2556,3400
 * NB : 1,1,11,27,39,40,43,43,44,48,67,115,131,120,195
 */
#define ATL_cGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 182) (nb_) = 1; \
   else if ((n_) < 261) (nb_) = 11; \
   else if ((n_) < 313) (nb_) = 27; \
   else if ((n_) < 366) (nb_) = 39; \
   else if ((n_) < 419) (nb_) = 40; \
   else if ((n_) < 604) (nb_) = 43; \
   else if ((n_) < 709) (nb_) = 44; \
   else if ((n_) < 815) (nb_) = 48; \
   else if ((n_) < 1079) (nb_) = 67; \
   else if ((n_) < 1501) (nb_) = 115; \
   else if ((n_) < 2134) (nb_) = 131; \
   else if ((n_) < 2978) (nb_) = 120; \
   else (nb_) = 195; \
}


#endif    /* end ifndef ATL_cGetNB_geqrf */
