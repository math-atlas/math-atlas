#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,48,64,96,176,352,528,560,576,608,704
 * N : 25,48,64,96,176,352,528,560,576,608,704
 * NB : 9,8,16,16,16,16,16,16,16,48,48
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 36) (nb_) = 9; \
   else if ((n_) < 56) (nb_) = 8; \
   else if ((n_) < 592) (nb_) = 16; \
   else (nb_) = 48; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
