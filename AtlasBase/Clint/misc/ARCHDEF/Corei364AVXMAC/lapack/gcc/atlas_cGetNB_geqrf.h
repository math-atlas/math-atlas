#ifndef ATL_cGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,86,148,271,394,456,518,1012,1506,2000
 * N : 25,86,148,271,394,456,518,1012,1506,2000
 * NB : 4,16,16,24,24,24,52,96,96,140
 */
#define ATL_cGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 55) (nb_) = 4; \
   else if ((n_) < 209) (nb_) = 16; \
   else if ((n_) < 487) (nb_) = 24; \
   else if ((n_) < 765) (nb_) = 52; \
   else if ((n_) < 1753) (nb_) = 96; \
   else (nb_) = 140; \
}


#endif    /* end ifndef ATL_cGetNB_geqrf */
