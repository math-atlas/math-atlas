#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,108,162,216,432,918,1890
 * N : 25,108,162,216,432,918,1890
 * NB : 7,12,54,54,54,54,54
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 66) (nb_) = 7; \
   else if ((n_) < 135) (nb_) = 12; \
   else (nb_) = 54; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
