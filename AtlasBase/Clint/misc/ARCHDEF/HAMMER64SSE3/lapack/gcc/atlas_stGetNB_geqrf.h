#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='LOWER'
 * M : 25,252,504,756,1092,2268,3360,4536
 * N : 25,252,504,756,1092,2268,3360,4536
 * NB : 4,84,28,84,84,84,168,168
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 138) (nb_) = 4; \
   else if ((n_) < 378) (nb_) = 84; \
   else if ((n_) < 630) (nb_) = 28; \
   else if ((n_) < 2814) (nb_) = 84; \
   else (nb_) = 168; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
