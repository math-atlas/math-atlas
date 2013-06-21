#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,112,168,224,448,672,784,952,1904,3808
 * N : 25,112,168,224,448,672,784,952,1904,3808
 * NB : 3,8,12,56,56,56,56,112,168,280
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 68) (nb_) = 3; \
   else if ((n_) < 140) (nb_) = 8; \
   else if ((n_) < 196) (nb_) = 12; \
   else if ((n_) < 868) (nb_) = 56; \
   else if ((n_) < 1428) (nb_) = 112; \
   else if ((n_) < 2856) (nb_) = 168; \
   else (nb_) = 280; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
