#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,72,108,144,324,504,576,612,684
 * N : 25,72,108,144,324,504,576,612,684
 * NB : 12,12,36,42,36,36,36,36,72
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 90) (nb_) = 12; \
   else if ((n_) < 126) (nb_) = 36; \
   else if ((n_) < 234) (nb_) = 42; \
   else if ((n_) < 648) (nb_) = 36; \
   else (nb_) = 72; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
