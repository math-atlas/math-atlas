#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,120,240,520,800,1080,2200
 * N : 25,120,240,520,800,1080,2200
 * NB : 7,40,40,40,40,80,80
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 72) (nb_) = 7; \
   else if ((n_) < 940) (nb_) = 40; \
   else (nb_) = 80; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
