#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,240,560,1120,1680,1920,2000,2080,2240
 * N : 25,240,560,1120,1680,1920,2000,2080,2240
 * NB : 5,20,20,40,80,40,80,160,160
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 132) (nb_) = 5; \
   else if ((n_) < 840) (nb_) = 20; \
   else if ((n_) < 1400) (nb_) = 40; \
   else if ((n_) < 1800) (nb_) = 80; \
   else if ((n_) < 1960) (nb_) = 40; \
   else if ((n_) < 2040) (nb_) = 80; \
   else (nb_) = 160; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
