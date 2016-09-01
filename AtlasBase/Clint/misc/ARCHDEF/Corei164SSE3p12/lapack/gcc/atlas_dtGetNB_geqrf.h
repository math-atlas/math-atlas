#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,87,149,211,273,521,1018,2012,4000
 * N : 25,87,149,211,273,521,1018,2012,4000
 * NB : 8,20,32,40,48,48,48,48,52
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 56) (nb_) = 8; \
   else if ((n_) < 118) (nb_) = 20; \
   else if ((n_) < 180) (nb_) = 32; \
   else if ((n_) < 242) (nb_) = 40; \
   else if ((n_) < 3006) (nb_) = 48; \
   else (nb_) = 52; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
