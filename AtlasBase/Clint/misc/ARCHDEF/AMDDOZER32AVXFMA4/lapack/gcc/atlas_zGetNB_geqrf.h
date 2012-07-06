#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,72,108,144,180,396,828,1656
 * N : 25,72,108,144,180,396,828,1656
 * NB : 3,6,12,12,36,36,36,36
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 48) (nb_) = 3; \
   else if ((n_) < 90) (nb_) = 6; \
   else if ((n_) < 162) (nb_) = 12; \
   else (nb_) = 36; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
