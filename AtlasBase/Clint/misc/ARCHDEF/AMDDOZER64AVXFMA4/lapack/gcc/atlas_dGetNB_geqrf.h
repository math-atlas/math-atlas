#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,80,120,160,240,280,320,680,1360,2760
 * N : 25,80,120,160,240,280,320,680,1360,2760
 * NB : 4,8,8,12,12,20,40,40,40,80
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 52) (nb_) = 4; \
   else if ((n_) < 140) (nb_) = 8; \
   else if ((n_) < 260) (nb_) = 12; \
   else if ((n_) < 300) (nb_) = 20; \
   else if ((n_) < 2060) (nb_) = 40; \
   else (nb_) = 80; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
