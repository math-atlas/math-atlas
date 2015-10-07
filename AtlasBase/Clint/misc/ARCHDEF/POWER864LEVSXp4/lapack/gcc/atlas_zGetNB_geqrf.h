#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,86,148,271,394,518,1012,2000
 * N : 25,86,148,271,394,518,1012,2000
 * NB : 4,24,24,24,64,68,72,126
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 55) (nb_) = 4; \
   else if ((n_) < 332) (nb_) = 24; \
   else if ((n_) < 456) (nb_) = 64; \
   else if ((n_) < 765) (nb_) = 68; \
   else if ((n_) < 1506) (nb_) = 72; \
   else (nb_) = 126; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
