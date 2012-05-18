#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,112,224,280,336,448,896,1792,3640
 * N : 25,112,224,280,336,448,896,1792,3640
 * NB : 12,24,24,16,56,56,56,56,56
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 68) (nb_) = 12; \
   else if ((n_) < 252) (nb_) = 24; \
   else if ((n_) < 308) (nb_) = 16; \
   else (nb_) = 56; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
