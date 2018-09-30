#ifndef ATL_ctGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,146,267,327,388,449,510,631,753,874,996,1482,1968,2454,2940,3183,3426,3669,3912,5856,7800
 * N : 25,146,267,327,388,449,510,631,753,874,996,1482,1968,2454,2940,3183,3426,3669,3912,5856,7800
 * NB : 1,1,35,35,36,48,51,47,63,64,67,68,83,83,84,92,96,96,99,115,227
 */
#define ATL_ctGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 206) (nb_) = 1; \
   else if ((n_) < 357) (nb_) = 35; \
   else if ((n_) < 418) (nb_) = 36; \
   else if ((n_) < 479) (nb_) = 48; \
   else if ((n_) < 570) (nb_) = 51; \
   else if ((n_) < 692) (nb_) = 47; \
   else if ((n_) < 813) (nb_) = 63; \
   else if ((n_) < 935) (nb_) = 64; \
   else if ((n_) < 1239) (nb_) = 67; \
   else if ((n_) < 1725) (nb_) = 68; \
   else if ((n_) < 2697) (nb_) = 83; \
   else if ((n_) < 3061) (nb_) = 84; \
   else if ((n_) < 3304) (nb_) = 92; \
   else if ((n_) < 3790) (nb_) = 96; \
   else if ((n_) < 4884) (nb_) = 99; \
   else if ((n_) < 6828) (nb_) = 115; \
   else (nb_) = 227; \
}


#endif    /* end ifndef ATL_ctGetNB_geqrf */
