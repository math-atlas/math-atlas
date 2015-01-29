#ifndef ATL_cGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,518,641,765,1012,1506,2000
 * N : 25,518,641,765,1012,1506,2000
 * NB : 24,24,24,76,76,96,200
 */
#define ATL_cGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 703) (nb_) = 24; \
   else if ((n_) < 1259) (nb_) = 76; \
   else if ((n_) < 1753) (nb_) = 96; \
   else (nb_) = 200; \
}


#endif    /* end ifndef ATL_cGetNB_geqrf */
