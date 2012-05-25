#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,112,280,336,448,616,1288
 * N : 25,112,280,336,448,616,1288
 * NB : 4,16,20,56,56,56,56
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 68) (nb_) = 4; \
   else if ((n_) < 196) (nb_) = 16; \
   else if ((n_) < 308) (nb_) = 20; \
   else (nb_) = 56; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
