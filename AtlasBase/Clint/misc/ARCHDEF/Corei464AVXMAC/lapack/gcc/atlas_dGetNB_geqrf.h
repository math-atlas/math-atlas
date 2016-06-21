#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,112,280,560,1120,2240,4536
 * N : 25,112,280,560,1120,2240,4536
 * NB : 5,56,56,56,56,112,168
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 68) (nb_) = 5; \
   else if ((n_) < 1680) (nb_) = 56; \
   else if ((n_) < 3388) (nb_) = 112; \
   else (nb_) = 168; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
