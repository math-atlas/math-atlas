#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='LOWER'
 * M : 25,104,156,208,416,520,624,884,1092,1352,1820,3640
 * N : 25,104,156,208,416,520,624,884,1092,1352,1820,3640
 * NB : 4,8,12,28,24,52,52,52,52,104,104,104
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 64) (nb_) = 4; \
   else if ((n_) < 130) (nb_) = 8; \
   else if ((n_) < 182) (nb_) = 12; \
   else if ((n_) < 312) (nb_) = 28; \
   else if ((n_) < 468) (nb_) = 24; \
   else if ((n_) < 1222) (nb_) = 52; \
   else (nb_) = 104; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
