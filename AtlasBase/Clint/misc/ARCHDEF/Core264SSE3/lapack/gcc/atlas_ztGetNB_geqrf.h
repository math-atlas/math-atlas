#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='LOWER'
 * M : 25,104,156,208,416,832,1664,3380
 * N : 25,104,156,208,416,832,1664,3380
 * NB : 5,4,16,52,52,52,104,104
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 64) (nb_) = 5; \
   else if ((n_) < 130) (nb_) = 4; \
   else if ((n_) < 182) (nb_) = 16; \
   else if ((n_) < 1248) (nb_) = 52; \
   else (nb_) = 104; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
