#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,120,280,560,1120,2240
 * N : 25,120,280,560,1120,2240
 * NB : 9,40,40,40,80,80
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 72) (nb_) = 9; \
   else if ((n_) < 840) (nb_) = 40; \
   else (nb_) = 80; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
