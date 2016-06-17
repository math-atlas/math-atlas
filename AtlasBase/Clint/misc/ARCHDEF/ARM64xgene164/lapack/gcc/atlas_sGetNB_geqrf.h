#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,240,320,400,560,1120,2320
 * N : 25,240,320,400,560,1120,2320
 * NB : 12,16,12,80,80,80,80
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 132) (nb_) = 12; \
   else if ((n_) < 280) (nb_) = 16; \
   else if ((n_) < 360) (nb_) = 12; \
   else (nb_) = 80; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
