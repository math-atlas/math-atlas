#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,96,192,432,528,576,624,864,1728,3456
 * N : 25,96,192,432,528,576,624,864,1728,3456
 * NB : 8,48,48,48,48,48,52,52,48,48
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 60) (nb_) = 8; \
   else if ((n_) < 600) (nb_) = 48; \
   else if ((n_) < 1296) (nb_) = 52; \
   else (nb_) = 48; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
