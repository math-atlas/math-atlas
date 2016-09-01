#ifndef ATL_cGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,86,148,209,271,394,518,1012,2000
 * N : 25,86,148,209,271,394,518,1012,2000
 * NB : 8,16,16,24,24,44,44,68,120
 */
#define ATL_cGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 55) (nb_) = 8; \
   else if ((n_) < 178) (nb_) = 16; \
   else if ((n_) < 332) (nb_) = 24; \
   else if ((n_) < 765) (nb_) = 44; \
   else if ((n_) < 1506) (nb_) = 68; \
   else (nb_) = 120; \
}


#endif    /* end ifndef ATL_cGetNB_geqrf */
