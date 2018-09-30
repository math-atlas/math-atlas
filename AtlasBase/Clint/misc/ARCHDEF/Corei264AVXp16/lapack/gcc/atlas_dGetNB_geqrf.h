#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,86,147,208,270,393,516,762,1008,1500,1992,2976,3960
 * N : 25,86,147,208,270,393,516,762,1008,1500,1992,2976,3960
 * NB : 1,1,15,18,35,39,51,59,99,96,131,171,195
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 116) (nb_) = 1; \
   else if ((n_) < 177) (nb_) = 15; \
   else if ((n_) < 239) (nb_) = 18; \
   else if ((n_) < 331) (nb_) = 35; \
   else if ((n_) < 454) (nb_) = 39; \
   else if ((n_) < 639) (nb_) = 51; \
   else if ((n_) < 885) (nb_) = 59; \
   else if ((n_) < 1254) (nb_) = 99; \
   else if ((n_) < 1746) (nb_) = 96; \
   else if ((n_) < 2484) (nb_) = 131; \
   else if ((n_) < 3468) (nb_) = 171; \
   else (nb_) = 195; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
