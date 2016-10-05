#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,149,211,273,521,583,645,769,1018,2012,4000
 * N : 25,149,211,273,521,583,645,769,1018,2012,4000
 * NB : 12,12,12,68,68,84,84,84,84,84,124
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 242) (nb_) = 12; \
   else if ((n_) < 552) (nb_) = 68; \
   else if ((n_) < 3006) (nb_) = 84; \
   else (nb_) = 124; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
