#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,80,160,200,240,320,680,1400,2840
 * N : 25,80,160,200,240,320,680,1400,2840
 * NB : 2,8,8,40,40,40,40,40,80
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 52) (nb_) = 2; \
   else if ((n_) < 180) (nb_) = 8; \
   else if ((n_) < 2120) (nb_) = 40; \
   else (nb_) = 80; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
