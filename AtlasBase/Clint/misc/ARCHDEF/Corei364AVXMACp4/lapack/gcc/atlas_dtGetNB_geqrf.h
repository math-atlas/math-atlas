#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,149,211,273,335,397,521,1018,1515,2012,4000
 * N : 25,149,211,273,335,397,521,1018,1515,2012,4000
 * NB : 12,12,12,48,48,76,76,76,80,132,144
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 242) (nb_) = 12; \
   else if ((n_) < 366) (nb_) = 48; \
   else if ((n_) < 1266) (nb_) = 76; \
   else if ((n_) < 1763) (nb_) = 80; \
   else if ((n_) < 3006) (nb_) = 132; \
   else (nb_) = 144; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
