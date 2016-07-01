#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,144,288,432,528,576,624,1296
 * N : 25,144,288,432,528,576,624,1296
 * NB : 4,24,8,8,8,8,48,48
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 84) (nb_) = 4; \
   else if ((n_) < 216) (nb_) = 24; \
   else if ((n_) < 600) (nb_) = 8; \
   else (nb_) = 48; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
