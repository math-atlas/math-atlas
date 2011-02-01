#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,156,208,312,624,780,832,936,1248,2496,2808,2964,3120,3744,4368,4680,4992,10000
 * N : 25,156,208,312,624,780,832,936,1248,2496,2808,2964,3120,3744,4368,4680,4992,10000
 * NB : 2,12,12,16,28,28,52,52,52,52,80,64,128,104,104,104,156,156
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 90) (nb_) = 2; \
   else if ((n_) < 260) (nb_) = 12; \
   else if ((n_) < 468) (nb_) = 16; \
   else if ((n_) < 806) (nb_) = 28; \
   else if ((n_) < 2652) (nb_) = 52; \
   else if ((n_) < 2886) (nb_) = 80; \
   else if ((n_) < 3042) (nb_) = 64; \
   else if ((n_) < 3432) (nb_) = 128; \
   else if ((n_) < 4836) (nb_) = 104; \
   else (nb_) = 156; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
