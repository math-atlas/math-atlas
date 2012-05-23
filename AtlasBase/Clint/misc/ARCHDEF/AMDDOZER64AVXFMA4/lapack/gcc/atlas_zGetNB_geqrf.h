#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,204,272,340,476,952,1904
 * N : 25,204,272,340,476,952,1904
 * NB : 5,20,68,68,68,68,68
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 114) (nb_) = 5; \
   else if ((n_) < 238) (nb_) = 20; \
   else (nb_) = 68; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
