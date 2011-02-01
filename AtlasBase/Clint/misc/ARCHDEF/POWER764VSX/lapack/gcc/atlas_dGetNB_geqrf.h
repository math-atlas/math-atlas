#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,104,208,416,624,676,728,884,1768,3588
 * N : 25,104,208,416,624,676,728,884,1768,3588
 * NB : 12,4,16,16,16,20,48,52,52,52
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 64) (nb_) = 12; \
   else if ((n_) < 156) (nb_) = 4; \
   else if ((n_) < 650) (nb_) = 16; \
   else if ((n_) < 702) (nb_) = 20; \
   else if ((n_) < 806) (nb_) = 48; \
   else (nb_) = 52; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
