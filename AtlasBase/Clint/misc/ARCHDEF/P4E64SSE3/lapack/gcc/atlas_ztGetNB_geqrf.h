#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,204,408,884,1292,1496,1632,1768
 * N : 25,204,408,884,1292,1496,1632,1768
 * NB : 9,68,68,68,68,68,68,136
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 114) (nb_) = 9; \
   else if ((n_) < 1700) (nb_) = 68; \
   else (nb_) = 136; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
