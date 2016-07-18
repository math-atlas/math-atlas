#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,144,192,288,624,768,864,960,1296,2592,5232
 * N : 25,144,192,288,624,768,864,960,1296,2592,5232
 * NB : 2,12,48,48,48,48,96,96,96,96,144
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 84) (nb_) = 2; \
   else if ((n_) < 168) (nb_) = 12; \
   else if ((n_) < 816) (nb_) = 48; \
   else if ((n_) < 3912) (nb_) = 96; \
   else (nb_) = 144; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
