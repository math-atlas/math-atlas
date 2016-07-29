#ifndef ATL_cGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,86,148,271,394,456,518,765,1012,2000
 * N : 25,86,148,271,394,456,518,765,1012,2000
 * NB : 4,24,24,24,40,48,48,96,96,116
 */
#define ATL_cGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 55) (nb_) = 4; \
   else if ((n_) < 332) (nb_) = 24; \
   else if ((n_) < 425) (nb_) = 40; \
   else if ((n_) < 641) (nb_) = 48; \
   else if ((n_) < 1506) (nb_) = 96; \
   else (nb_) = 116; \
}


#endif    /* end ifndef ATL_cGetNB_geqrf */
