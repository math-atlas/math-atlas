#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,107,190,273,356,522,688,854,1020,1186,1352,2016,2680
 * N : 25,107,190,273,356,522,688,854,1020,1186,1352,2016,2680
 * NB : 1,1,3,3,4,4,7,7,8,8,11,11,24
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 148) (nb_) = 1; \
   else if ((n_) < 314) (nb_) = 3; \
   else if ((n_) < 605) (nb_) = 4; \
   else if ((n_) < 937) (nb_) = 7; \
   else if ((n_) < 1269) (nb_) = 8; \
   else if ((n_) < 2348) (nb_) = 11; \
   else (nb_) = 24; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
