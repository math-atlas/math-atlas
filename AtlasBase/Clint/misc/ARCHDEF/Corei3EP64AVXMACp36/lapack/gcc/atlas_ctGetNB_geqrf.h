#ifndef ATL_ctGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,180,336,375,414,453,492,531,570,609,648,959,1271,1894,2206,2518,3765,5012,10000
 * N : 25,180,336,375,414,453,492,531,570,609,648,959,1271,1894,2206,2518,3765,5012,10000
 * NB : 1,1,11,39,43,47,51,59,63,64,67,95,99,96,112,131,128,448,448
 */
#define ATL_ctGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 258) (nb_) = 1; \
   else if ((n_) < 355) (nb_) = 11; \
   else if ((n_) < 394) (nb_) = 39; \
   else if ((n_) < 433) (nb_) = 43; \
   else if ((n_) < 472) (nb_) = 47; \
   else if ((n_) < 511) (nb_) = 51; \
   else if ((n_) < 550) (nb_) = 59; \
   else if ((n_) < 589) (nb_) = 63; \
   else if ((n_) < 628) (nb_) = 64; \
   else if ((n_) < 803) (nb_) = 67; \
   else if ((n_) < 1115) (nb_) = 95; \
   else if ((n_) < 1582) (nb_) = 99; \
   else if ((n_) < 2050) (nb_) = 96; \
   else if ((n_) < 2362) (nb_) = 112; \
   else if ((n_) < 3141) (nb_) = 131; \
   else if ((n_) < 4388) (nb_) = 128; \
   else (nb_) = 448; \
}


#endif    /* end ifndef ATL_ctGetNB_geqrf */
