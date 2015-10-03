#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,148,209,271,394,518,1012,1506,2000
 * N : 25,148,209,271,394,518,1012,1506,2000
 * NB : 12,12,24,24,24,48,72,72,144
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 178) (nb_) = 12; \
   else if ((n_) < 456) (nb_) = 24; \
   else if ((n_) < 765) (nb_) = 48; \
   else if ((n_) < 1753) (nb_) = 72; \
   else (nb_) = 144; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
