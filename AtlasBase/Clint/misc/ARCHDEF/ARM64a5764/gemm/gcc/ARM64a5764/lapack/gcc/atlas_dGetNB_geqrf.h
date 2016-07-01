#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,80,120,200,440,880,1800
 * N : 25,80,120,200,440,880,1800
 * NB : 12,20,40,40,40,40,80
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 52) (nb_) = 12; \
   else if ((n_) < 100) (nb_) = 20; \
   else if ((n_) < 1340) (nb_) = 40; \
   else (nb_) = 80; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
