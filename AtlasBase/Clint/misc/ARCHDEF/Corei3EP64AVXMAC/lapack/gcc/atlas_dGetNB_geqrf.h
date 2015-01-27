#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,148,209,271,332,394,518,1012,1506,2000
 * N : 25,148,209,271,332,394,518,1012,1506,2000
 * NB : 8,8,10,10,24,24,48,60,60,108
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 178) (nb_) = 8; \
   else if ((n_) < 301) (nb_) = 10; \
   else if ((n_) < 456) (nb_) = 24; \
   else if ((n_) < 765) (nb_) = 48; \
   else if ((n_) < 1753) (nb_) = 60; \
   else (nb_) = 108; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
