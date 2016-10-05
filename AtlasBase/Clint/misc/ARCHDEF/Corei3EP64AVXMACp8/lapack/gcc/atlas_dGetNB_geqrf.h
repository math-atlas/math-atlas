#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,86,148,271,518,641,765,1012,2000
 * N : 25,86,148,271,518,641,765,1012,2000
 * NB : 12,12,20,24,36,36,84,84,144
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 117) (nb_) = 12; \
   else if ((n_) < 209) (nb_) = 20; \
   else if ((n_) < 394) (nb_) = 24; \
   else if ((n_) < 703) (nb_) = 36; \
   else if ((n_) < 1506) (nb_) = 84; \
   else (nb_) = 144; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
