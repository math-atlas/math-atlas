#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,88,120,152,184,216,248,280,408,472,536,664,792,920,1048,1560,2072,3096,3608,4120
 * N : 25,88,120,152,184,216,248,280,408,472,536,664,792,920,1048,1560,2072,3096,3608,4120
 * NB : 1,4,10,11,13,14,14,15,13,13,23,23,24,24,27,39,43,42,46,192
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 56) (nb_) = 1; \
   else if ((n_) < 104) (nb_) = 4; \
   else if ((n_) < 136) (nb_) = 10; \
   else if ((n_) < 168) (nb_) = 11; \
   else if ((n_) < 200) (nb_) = 13; \
   else if ((n_) < 264) (nb_) = 14; \
   else if ((n_) < 344) (nb_) = 15; \
   else if ((n_) < 504) (nb_) = 13; \
   else if ((n_) < 728) (nb_) = 23; \
   else if ((n_) < 984) (nb_) = 24; \
   else if ((n_) < 1304) (nb_) = 27; \
   else if ((n_) < 1816) (nb_) = 39; \
   else if ((n_) < 2584) (nb_) = 43; \
   else if ((n_) < 3352) (nb_) = 42; \
   else if ((n_) < 3864) (nb_) = 46; \
   else (nb_) = 192; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
