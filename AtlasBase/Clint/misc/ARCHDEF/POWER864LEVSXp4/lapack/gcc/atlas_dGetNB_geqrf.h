#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,86,148,271,332,394,518,1012,2000
 * N : 25,86,148,271,332,394,518,1012,2000
 * NB : 4,24,24,24,48,48,48,72,108
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 55) (nb_) = 4; \
   else if ((n_) < 301) (nb_) = 24; \
   else if ((n_) < 765) (nb_) = 48; \
   else if ((n_) < 1506) (nb_) = 72; \
   else (nb_) = 108; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
