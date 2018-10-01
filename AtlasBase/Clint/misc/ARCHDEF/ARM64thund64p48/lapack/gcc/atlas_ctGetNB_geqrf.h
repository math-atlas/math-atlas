#ifndef ATL_ctGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,65,106,146,187,227,268,309,350,431,513,594,676,1002,1328,1491,1654,1817,1980,2306,2632,3936,5240
 * N : 25,65,106,146,187,227,268,309,350,431,513,594,676,1002,1328,1491,1654,1817,1980,2306,2632,3936,5240
 * NB : 1,4,7,8,9,9,10,10,11,11,12,12,15,15,19,16,31,31,43,43,59,59,224
 */
#define ATL_ctGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 45) (nb_) = 1; \
   else if ((n_) < 85) (nb_) = 4; \
   else if ((n_) < 126) (nb_) = 7; \
   else if ((n_) < 166) (nb_) = 8; \
   else if ((n_) < 247) (nb_) = 9; \
   else if ((n_) < 329) (nb_) = 10; \
   else if ((n_) < 472) (nb_) = 11; \
   else if ((n_) < 635) (nb_) = 12; \
   else if ((n_) < 1165) (nb_) = 15; \
   else if ((n_) < 1409) (nb_) = 19; \
   else if ((n_) < 1572) (nb_) = 16; \
   else if ((n_) < 1898) (nb_) = 31; \
   else if ((n_) < 2469) (nb_) = 43; \
   else if ((n_) < 4588) (nb_) = 59; \
   else (nb_) = 224; \
}


#endif    /* end ifndef ATL_ctGetNB_geqrf */
