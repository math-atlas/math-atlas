#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,112,168,224,280,616,728,784,840,896,1232,2464,4984,10000
 * N : 25,112,168,224,280,616,728,784,840,896,1232,2464,4984,10000
 * NB : 2,24,16,56,80,12,8,104,8,108,104,88,88,48
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 68) (nb_) = 2; \
   else if ((n_) < 140) (nb_) = 24; \
   else if ((n_) < 196) (nb_) = 16; \
   else if ((n_) < 252) (nb_) = 56; \
   else if ((n_) < 448) (nb_) = 80; \
   else if ((n_) < 672) (nb_) = 12; \
   else if ((n_) < 756) (nb_) = 8; \
   else if ((n_) < 812) (nb_) = 104; \
   else if ((n_) < 868) (nb_) = 8; \
   else if ((n_) < 1064) (nb_) = 108; \
   else if ((n_) < 1848) (nb_) = 104; \
   else if ((n_) < 7492) (nb_) = 88; \
   else (nb_) = 48; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
