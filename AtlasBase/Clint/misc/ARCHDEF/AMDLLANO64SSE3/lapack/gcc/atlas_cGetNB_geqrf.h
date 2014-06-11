#ifndef ATL_cGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,92,160,296,568,1112,2200
 * N : 25,92,160,296,568,1112,2200
 * NB : 8,12,24,24,48,60,100
 */
#define ATL_cGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 58) (nb_) = 8; \
   else if ((n_) < 126) (nb_) = 12; \
   else if ((n_) < 432) (nb_) = 24; \
   else if ((n_) < 840) (nb_) = 48; \
   else if ((n_) < 1656) (nb_) = 60; \
   else (nb_) = 100; \
}


#endif    /* end ifndef ATL_cGetNB_geqrf */
