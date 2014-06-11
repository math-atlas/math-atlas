#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,67,110,152,195,280,323,366,708,1392,2760
 * N : 25,67,110,152,195,280,323,366,708,1392,2760
 * NB : 6,6,12,12,18,24,24,48,48,84,102
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 88) (nb_) = 6; \
   else if ((n_) < 173) (nb_) = 12; \
   else if ((n_) < 237) (nb_) = 18; \
   else if ((n_) < 344) (nb_) = 24; \
   else if ((n_) < 1050) (nb_) = 48; \
   else if ((n_) < 2076) (nb_) = 84; \
   else (nb_) = 102; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
