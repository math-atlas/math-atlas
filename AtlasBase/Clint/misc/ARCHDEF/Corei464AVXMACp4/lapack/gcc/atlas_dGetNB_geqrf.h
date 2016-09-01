#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,86,148,271,394,518,641,765,1012,2000
 * N : 25,86,148,271,394,518,641,765,1012,2000
 * NB : 8,16,24,24,24,36,48,72,72,148
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 55) (nb_) = 8; \
   else if ((n_) < 117) (nb_) = 16; \
   else if ((n_) < 456) (nb_) = 24; \
   else if ((n_) < 579) (nb_) = 36; \
   else if ((n_) < 703) (nb_) = 48; \
   else if ((n_) < 1506) (nb_) = 72; \
   else (nb_) = 148; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
