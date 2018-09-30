#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,68,111,154,197,283,370,456,543,629,716,889,1062,1235,1408,2100,2792,4176,5560
 * N : 25,68,111,154,197,283,370,456,543,629,716,889,1062,1235,1408,2100,2792,4176,5560
 * NB : 1,4,7,8,15,19,35,32,55,55,67,75,79,95,99,115,163,160,195
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 46) (nb_) = 1; \
   else if ((n_) < 89) (nb_) = 4; \
   else if ((n_) < 132) (nb_) = 7; \
   else if ((n_) < 175) (nb_) = 8; \
   else if ((n_) < 240) (nb_) = 15; \
   else if ((n_) < 326) (nb_) = 19; \
   else if ((n_) < 413) (nb_) = 35; \
   else if ((n_) < 499) (nb_) = 32; \
   else if ((n_) < 672) (nb_) = 55; \
   else if ((n_) < 802) (nb_) = 67; \
   else if ((n_) < 975) (nb_) = 75; \
   else if ((n_) < 1148) (nb_) = 79; \
   else if ((n_) < 1321) (nb_) = 95; \
   else if ((n_) < 1754) (nb_) = 99; \
   else if ((n_) < 2446) (nb_) = 115; \
   else if ((n_) < 3484) (nb_) = 163; \
   else if ((n_) < 4868) (nb_) = 160; \
   else (nb_) = 195; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
