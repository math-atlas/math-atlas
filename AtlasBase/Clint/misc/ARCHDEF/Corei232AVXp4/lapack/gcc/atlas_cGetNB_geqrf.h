#ifndef ATL_cGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,518,641,703,765,888,1012,2000
 * N : 25,518,641,703,765,888,1012,2000
 * NB : 24,28,28,28,80,80,84,112
 */
#define ATL_cGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 271) (nb_) = 24; \
   else if ((n_) < 734) (nb_) = 28; \
   else if ((n_) < 950) (nb_) = 80; \
   else if ((n_) < 1506) (nb_) = 84; \
   else (nb_) = 112; \
}


#endif    /* end ifndef ATL_cGetNB_geqrf */
