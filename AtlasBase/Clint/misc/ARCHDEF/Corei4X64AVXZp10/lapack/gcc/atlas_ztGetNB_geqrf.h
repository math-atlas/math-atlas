#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,90,156,221,287,418,550,813,1076,1339,1602,1865,2128,4232,6336,8440
 * N : 25,90,156,221,287,418,550,813,1076,1339,1602,1865,2128,4232,6336,8440
 * NB : 1,1,35,35,43,43,51,59,67,67,115,123,288,288,432,576
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 123) (nb_) = 1; \
   else if ((n_) < 254) (nb_) = 35; \
   else if ((n_) < 484) (nb_) = 43; \
   else if ((n_) < 681) (nb_) = 51; \
   else if ((n_) < 944) (nb_) = 59; \
   else if ((n_) < 1470) (nb_) = 67; \
   else if ((n_) < 1733) (nb_) = 115; \
   else if ((n_) < 1996) (nb_) = 123; \
   else if ((n_) < 5284) (nb_) = 288; \
   else if ((n_) < 7388) (nb_) = 432; \
   else (nb_) = 576; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
