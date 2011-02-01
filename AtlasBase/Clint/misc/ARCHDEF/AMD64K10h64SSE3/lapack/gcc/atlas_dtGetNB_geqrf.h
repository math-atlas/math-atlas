#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='LOWER'
 * M : 25,112,168,224,280,616,896,1232,2464,4928
 * N : 25,112,168,224,280,616,896,1232,2464,4928
 * NB : 9,16,14,56,56,56,112,112,112,112
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 68) (nb_) = 9; \
   else if ((n_) < 140) (nb_) = 16; \
   else if ((n_) < 196) (nb_) = 14; \
   else if ((n_) < 756) (nb_) = 56; \
   else (nb_) = 112; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
