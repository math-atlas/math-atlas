#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,84,143,202,262,321,381,440,500,738,857,916,976,1452,1690,1809,1868,1928,3832,7640
 * N : 25,84,143,202,262,321,381,440,500,738,857,916,976,1452,1690,1809,1868,1928,3832,7640
 * NB : 12,16,16,24,24,32,40,40,44,44,56,56,80,80,80,84,84,136,168,168
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 54) (nb_) = 12; \
   else if ((n_) < 172) (nb_) = 16; \
   else if ((n_) < 291) (nb_) = 24; \
   else if ((n_) < 351) (nb_) = 32; \
   else if ((n_) < 470) (nb_) = 40; \
   else if ((n_) < 797) (nb_) = 44; \
   else if ((n_) < 946) (nb_) = 56; \
   else if ((n_) < 1749) (nb_) = 80; \
   else if ((n_) < 1898) (nb_) = 84; \
   else if ((n_) < 2880) (nb_) = 136; \
   else (nb_) = 168; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
