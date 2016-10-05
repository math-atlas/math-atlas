#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,86,148,209,271,394,518,1012,2000
 * N : 25,86,148,209,271,394,518,1012,2000
 * NB : 8,20,20,20,28,28,40,52,52
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 55) (nb_) = 8; \
   else if ((n_) < 240) (nb_) = 20; \
   else if ((n_) < 456) (nb_) = 28; \
   else if ((n_) < 765) (nb_) = 40; \
   else (nb_) = 52; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
