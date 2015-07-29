#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,87,149,211,273,397,459,521,583,645,769,1018,2012,4000
 * N : 25,87,149,211,273,397,459,521,583,645,769,1018,2012,4000
 * NB : 4,24,24,24,28,28,30,30,36,36,36,36,40,48
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 56) (nb_) = 4; \
   else if ((n_) < 242) (nb_) = 24; \
   else if ((n_) < 428) (nb_) = 28; \
   else if ((n_) < 552) (nb_) = 30; \
   else if ((n_) < 1515) (nb_) = 36; \
   else if ((n_) < 3006) (nb_) = 40; \
   else (nb_) = 48; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
