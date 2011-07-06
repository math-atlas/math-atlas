#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,108,252,540,792,1080,2160
 * N : 25,108,252,540,792,1080,2160
 * NB : 9,36,36,36,36,72,72
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 66) (nb_) = 9; \
   else if ((n_) < 936) (nb_) = 36; \
   else (nb_) = 72; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
