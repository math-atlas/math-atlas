#ifndef ATL_cGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,70,115,205,295,386,476,567,657,702,748,1472,2920
 * N : 25,70,115,205,295,386,476,567,657,702,748,1472,2920
 * NB : 4,8,24,24,24,32,44,44,60,60,88,88,168
 */
#define ATL_cGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 47) (nb_) = 4; \
   else if ((n_) < 92) (nb_) = 8; \
   else if ((n_) < 340) (nb_) = 24; \
   else if ((n_) < 431) (nb_) = 32; \
   else if ((n_) < 612) (nb_) = 44; \
   else if ((n_) < 725) (nb_) = 60; \
   else if ((n_) < 2196) (nb_) = 88; \
   else (nb_) = 168; \
}


#endif    /* end ifndef ATL_cGetNB_geqrf */
