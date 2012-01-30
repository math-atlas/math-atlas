#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,88,220,264,308,352,396,484,748,1012,2024,4048
 * N : 25,88,220,264,308,352,396,484,748,1012,2024,4048
 * NB : 3,88,88,96,88,104,72,112,88,120,112,88
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 56) (nb_) = 3; \
   else if ((n_) < 242) (nb_) = 88; \
   else if ((n_) < 286) (nb_) = 96; \
   else if ((n_) < 330) (nb_) = 88; \
   else if ((n_) < 374) (nb_) = 104; \
   else if ((n_) < 440) (nb_) = 72; \
   else if ((n_) < 616) (nb_) = 112; \
   else if ((n_) < 880) (nb_) = 88; \
   else if ((n_) < 1518) (nb_) = 120; \
   else if ((n_) < 3036) (nb_) = 112; \
   else (nb_) = 88; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
