#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,518,641,703,765,1012,1506,2000
 * N : 25,518,641,703,765,1012,1506,2000
 * NB : 24,28,28,48,80,80,80,112
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 271) (nb_) = 24; \
   else if ((n_) < 672) (nb_) = 28; \
   else if ((n_) < 734) (nb_) = 48; \
   else if ((n_) < 1753) (nb_) = 80; \
   else (nb_) = 112; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
