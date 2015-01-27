#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,86,148,209,271,518,641,765,1012,1506,2000
 * N : 25,86,148,209,271,518,641,765,1012,1506,2000
 * NB : 12,12,16,24,24,28,28,60,60,60,100
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 117) (nb_) = 12; \
   else if ((n_) < 178) (nb_) = 16; \
   else if ((n_) < 394) (nb_) = 24; \
   else if ((n_) < 703) (nb_) = 28; \
   else if ((n_) < 1753) (nb_) = 60; \
   else (nb_) = 100; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
