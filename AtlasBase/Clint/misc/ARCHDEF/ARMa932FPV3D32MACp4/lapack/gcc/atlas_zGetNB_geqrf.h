#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,86,148,271,394,518,1012,2000
 * N : 25,86,148,271,394,518,1012,2000
 * NB : 4,24,24,24,30,30,30,30
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 55) (nb_) = 4; \
   else if ((n_) < 332) (nb_) = 24; \
   else (nb_) = 30; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
