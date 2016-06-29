#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,216,432,480,504,528,552,576,648,864,1728
 * N : 25,216,432,480,504,528,552,576,648,864,1728
 * NB : 12,12,24,24,24,48,48,72,72,72,96
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 324) (nb_) = 12; \
   else if ((n_) < 516) (nb_) = 24; \
   else if ((n_) < 564) (nb_) = 48; \
   else if ((n_) < 1296) (nb_) = 72; \
   else (nb_) = 96; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
