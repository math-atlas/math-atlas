#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,112,280,560,1120,1680,2240
 * N : 25,112,280,560,1120,1680,2240
 * NB : 9,56,56,56,56,56,112
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 68) (nb_) = 9; \
   else if ((n_) < 1960) (nb_) = 56; \
   else (nb_) = 112; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
