#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,95,166,308,592,876,1160
 * N : 25,95,166,308,592,876,1160
 * NB : 8,10,24,24,40,40,80
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 60) (nb_) = 8; \
   else if ((n_) < 130) (nb_) = 10; \
   else if ((n_) < 450) (nb_) = 24; \
   else if ((n_) < 1018) (nb_) = 40; \
   else (nb_) = 80; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
