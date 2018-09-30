#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,93,162,196,231,265,300,369,438,472,507,541,576,852,1128,1404,1680,1956,2232,3336,4440
 * N : 25,93,162,196,231,265,300,369,438,472,507,541,576,852,1128,1404,1680,1956,2232,3336,4440
 * NB : 1,1,15,15,16,24,27,27,28,32,34,34,35,59,83,83,84,108,131,179,227
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 127) (nb_) = 1; \
   else if ((n_) < 213) (nb_) = 15; \
   else if ((n_) < 248) (nb_) = 16; \
   else if ((n_) < 282) (nb_) = 24; \
   else if ((n_) < 403) (nb_) = 27; \
   else if ((n_) < 455) (nb_) = 28; \
   else if ((n_) < 489) (nb_) = 32; \
   else if ((n_) < 558) (nb_) = 34; \
   else if ((n_) < 714) (nb_) = 35; \
   else if ((n_) < 990) (nb_) = 59; \
   else if ((n_) < 1542) (nb_) = 83; \
   else if ((n_) < 1818) (nb_) = 84; \
   else if ((n_) < 2094) (nb_) = 108; \
   else if ((n_) < 2784) (nb_) = 131; \
   else if ((n_) < 3888) (nb_) = 179; \
   else (nb_) = 227; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
