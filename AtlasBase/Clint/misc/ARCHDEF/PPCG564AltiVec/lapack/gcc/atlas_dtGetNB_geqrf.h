#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,168,224,336,728,784,840,896,1120,1288,1400,1456,1512,2296,2688,2856,2968,3080
 * N : 25,168,224,336,728,784,840,896,1120,1288,1400,1456,1512,2296,2688,2856,2968,3080
 * NB : 2,12,56,56,56,68,56,100,104,96,56,56,168,168,168,168,224,224
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 96) (nb_) = 2; \
   else if ((n_) < 196) (nb_) = 12; \
   else if ((n_) < 756) (nb_) = 56; \
   else if ((n_) < 812) (nb_) = 68; \
   else if ((n_) < 868) (nb_) = 56; \
   else if ((n_) < 1008) (nb_) = 100; \
   else if ((n_) < 1204) (nb_) = 104; \
   else if ((n_) < 1344) (nb_) = 96; \
   else if ((n_) < 1484) (nb_) = 56; \
   else if ((n_) < 2912) (nb_) = 168; \
   else (nb_) = 224; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
