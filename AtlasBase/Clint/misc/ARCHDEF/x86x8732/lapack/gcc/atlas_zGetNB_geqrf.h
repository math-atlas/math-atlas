#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,132,308,352,440,616,1232
 * N : 25,132,308,352,440,616,1232
 * NB : 4,12,12,44,44,44,44
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 78) (nb_) = 4; \
   else if ((n_) < 330) (nb_) = 12; \
   else (nb_) = 44; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
