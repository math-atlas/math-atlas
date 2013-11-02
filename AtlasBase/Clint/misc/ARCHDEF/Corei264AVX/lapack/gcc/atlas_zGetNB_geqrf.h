#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,92,160,296,432,568,1112,2200
 * N : 25,92,160,296,432,568,1112,2200
 * NB : 12,12,24,24,24,44,72,96
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 126) (nb_) = 12; \
   else if ((n_) < 500) (nb_) = 24; \
   else if ((n_) < 840) (nb_) = 44; \
   else if ((n_) < 1656) (nb_) = 72; \
   else (nb_) = 96; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
