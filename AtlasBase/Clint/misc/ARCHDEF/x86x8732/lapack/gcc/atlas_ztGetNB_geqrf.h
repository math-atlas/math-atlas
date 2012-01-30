#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,132,308,616,1232,2508
 * N : 25,132,308,616,1232,2508
 * NB : 4,44,44,44,56,44
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 78) (nb_) = 4; \
   else if ((n_) < 924) (nb_) = 44; \
   else if ((n_) < 1870) (nb_) = 56; \
   else (nb_) = 44; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
