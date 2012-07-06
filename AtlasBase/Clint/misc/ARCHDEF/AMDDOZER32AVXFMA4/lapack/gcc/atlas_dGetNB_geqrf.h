#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,72,144,216,324,648,972,1332,2664
 * N : 25,72,144,216,324,648,972,1332,2664
 * NB : 2,12,12,36,36,36,36,72,72
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 48) (nb_) = 2; \
   else if ((n_) < 180) (nb_) = 12; \
   else if ((n_) < 1152) (nb_) = 36; \
   else (nb_) = 72; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
