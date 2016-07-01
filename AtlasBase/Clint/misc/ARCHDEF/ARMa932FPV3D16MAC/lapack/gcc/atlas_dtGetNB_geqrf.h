#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,72,108,144,324,684,756,792,864,1044,1404
 * N : 25,72,108,144,324,684,756,792,864,1044,1404
 * NB : 4,6,6,30,30,24,24,72,72,72,72
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 48) (nb_) = 4; \
   else if ((n_) < 126) (nb_) = 6; \
   else if ((n_) < 504) (nb_) = 30; \
   else if ((n_) < 774) (nb_) = 24; \
   else (nb_) = 72; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
