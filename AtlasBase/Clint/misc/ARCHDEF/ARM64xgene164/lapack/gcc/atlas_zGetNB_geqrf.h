#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,112,280,616,896,1064,1232
 * N : 25,112,280,616,896,1064,1232
 * NB : 12,56,20,20,28,28,112
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 68) (nb_) = 12; \
   else if ((n_) < 196) (nb_) = 56; \
   else if ((n_) < 756) (nb_) = 20; \
   else if ((n_) < 1148) (nb_) = 28; \
   else (nb_) = 112; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
