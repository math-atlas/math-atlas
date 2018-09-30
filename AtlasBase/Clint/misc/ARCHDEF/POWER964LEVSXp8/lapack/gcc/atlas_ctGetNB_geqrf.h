#ifndef ATL_ctGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,71,118,165,212,259,306,353,400,588,776,964,1152,1340,1528,2280,2656,3032,4536,6040
 * N : 25,71,118,165,212,259,306,353,400,588,776,964,1152,1340,1528,2280,2656,3032,4536,6040
 * NB : 1,9,11,12,15,19,21,22,23,23,35,36,47,47,51,48,48,83,80,99
 */
#define ATL_ctGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 48) (nb_) = 1; \
   else if ((n_) < 94) (nb_) = 9; \
   else if ((n_) < 141) (nb_) = 11; \
   else if ((n_) < 188) (nb_) = 12; \
   else if ((n_) < 235) (nb_) = 15; \
   else if ((n_) < 282) (nb_) = 19; \
   else if ((n_) < 329) (nb_) = 21; \
   else if ((n_) < 376) (nb_) = 22; \
   else if ((n_) < 682) (nb_) = 23; \
   else if ((n_) < 870) (nb_) = 35; \
   else if ((n_) < 1058) (nb_) = 36; \
   else if ((n_) < 1434) (nb_) = 47; \
   else if ((n_) < 1904) (nb_) = 51; \
   else if ((n_) < 2844) (nb_) = 48; \
   else if ((n_) < 3784) (nb_) = 83; \
   else if ((n_) < 5288) (nb_) = 80; \
   else (nb_) = 99; \
}


#endif    /* end ifndef ATL_ctGetNB_geqrf */
