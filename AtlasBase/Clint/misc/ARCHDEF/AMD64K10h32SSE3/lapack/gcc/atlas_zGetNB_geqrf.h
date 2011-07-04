#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,198,264,396,792,1584
 * N : 25,198,264,396,792,1584
 * NB : 9,12,66,66,66,66
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 111) (nb_) = 9; \
   else if ((n_) < 231) (nb_) = 12; \
   else (nb_) = 66; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
