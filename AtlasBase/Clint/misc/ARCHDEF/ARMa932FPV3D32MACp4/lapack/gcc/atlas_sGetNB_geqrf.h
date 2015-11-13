#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,86,148,271,394,456,518,1012,2000
 * N : 25,86,148,271,394,456,518,1012,2000
 * NB : 4,24,24,24,24,24,48,60,60
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 55) (nb_) = 4; \
   else if ((n_) < 487) (nb_) = 24; \
   else if ((n_) < 765) (nb_) = 48; \
   else (nb_) = 60; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
