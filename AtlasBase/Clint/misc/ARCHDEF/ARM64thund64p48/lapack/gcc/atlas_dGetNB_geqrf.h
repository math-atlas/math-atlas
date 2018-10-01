#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,85,145,205,266,387,508,750,871,992,1234,1476,1718,1960
 * N : 25,85,145,205,266,387,508,750,871,992,1234,1476,1718,1960
 * NB : 1,11,15,23,27,31,51,46,78,83,84,90,122,131
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 55) (nb_) = 1; \
   else if ((n_) < 115) (nb_) = 11; \
   else if ((n_) < 175) (nb_) = 15; \
   else if ((n_) < 235) (nb_) = 23; \
   else if ((n_) < 326) (nb_) = 27; \
   else if ((n_) < 447) (nb_) = 31; \
   else if ((n_) < 629) (nb_) = 51; \
   else if ((n_) < 810) (nb_) = 46; \
   else if ((n_) < 931) (nb_) = 78; \
   else if ((n_) < 1113) (nb_) = 83; \
   else if ((n_) < 1355) (nb_) = 84; \
   else if ((n_) < 1597) (nb_) = 90; \
   else if ((n_) < 1839) (nb_) = 122; \
   else (nb_) = 131; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
