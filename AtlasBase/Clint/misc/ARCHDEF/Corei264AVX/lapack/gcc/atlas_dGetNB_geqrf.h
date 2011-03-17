#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,60,90,150,330,660,990,1320,2640
 * N : 25,60,90,150,330,660,990,1320,2640
 * NB : 9,12,30,30,30,30,90,90,90
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 42) (nb_) = 9; \
   else if ((n_) < 75) (nb_) = 12; \
   else if ((n_) < 825) (nb_) = 30; \
   else (nb_) = 90; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
