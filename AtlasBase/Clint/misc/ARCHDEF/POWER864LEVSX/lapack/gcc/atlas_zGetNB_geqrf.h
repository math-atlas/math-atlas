#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,216,432,864,1728
 * N : 25,216,432,864,1728
 * NB : 12,12,24,36,48
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 324) (nb_) = 12; \
   else if ((n_) < 648) (nb_) = 24; \
   else if ((n_) < 1296) (nb_) = 36; \
   else (nb_) = 48; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
