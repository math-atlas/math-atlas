#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,72,144,324,396,504,576,684,1008,1368,1404,1440,1512,1692,2052,2736,5508
 * N : 25,72,144,324,396,504,576,684,1008,1368,1404,1440,1512,1692,2052,2736,5508
 * NB : 4,8,8,12,36,36,36,112,108,144,216,216,216,216,216,216,216
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 48) (nb_) = 4; \
   else if ((n_) < 234) (nb_) = 8; \
   else if ((n_) < 360) (nb_) = 12; \
   else if ((n_) < 630) (nb_) = 36; \
   else if ((n_) < 846) (nb_) = 112; \
   else if ((n_) < 1188) (nb_) = 108; \
   else if ((n_) < 1386) (nb_) = 144; \
   else (nb_) = 216; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
