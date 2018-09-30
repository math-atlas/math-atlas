#ifndef ATL_ctGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,122,220,268,317,366,415,610,806,1197,1588,2370,3152,4716,6280
 * N : 25,122,220,268,317,366,415,610,806,1197,1588,2370,3152,4716,6280
 * NB : 1,1,11,35,39,40,43,43,51,51,83,80,99,111,227
 */
#define ATL_ctGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 171) (nb_) = 1; \
   else if ((n_) < 244) (nb_) = 11; \
   else if ((n_) < 292) (nb_) = 35; \
   else if ((n_) < 341) (nb_) = 39; \
   else if ((n_) < 390) (nb_) = 40; \
   else if ((n_) < 708) (nb_) = 43; \
   else if ((n_) < 1392) (nb_) = 51; \
   else if ((n_) < 1979) (nb_) = 83; \
   else if ((n_) < 2761) (nb_) = 80; \
   else if ((n_) < 3934) (nb_) = 99; \
   else if ((n_) < 5498) (nb_) = 111; \
   else (nb_) = 227; \
}


#endif    /* end ifndef ATL_ctGetNB_geqrf */
