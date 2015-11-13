#ifndef ATL_cGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,86,148,271,518,1012,2000
 * N : 25,86,148,271,518,1012,2000
 * NB : 4,24,24,24,24,48,60
 */
#define ATL_cGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 55) (nb_) = 4; \
   else if ((n_) < 765) (nb_) = 24; \
   else if ((n_) < 1506) (nb_) = 48; \
   else (nb_) = 60; \
}


#endif    /* end ifndef ATL_cGetNB_geqrf */
