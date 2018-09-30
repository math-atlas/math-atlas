#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,66,107,148,190,231,273,314,356,439,522,563,605,646,688,1020,1186,1352,2016,2680
 * N : 25,66,107,148,190,231,273,314,356,439,522,563,605,646,688,1020,1186,1352,2016,2680
 * NB : 1,5,11,12,23,23,24,24,35,35,36,36,48,48,51,48,72,83,72,131
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 45) (nb_) = 1; \
   else if ((n_) < 86) (nb_) = 5; \
   else if ((n_) < 127) (nb_) = 11; \
   else if ((n_) < 169) (nb_) = 12; \
   else if ((n_) < 252) (nb_) = 23; \
   else if ((n_) < 335) (nb_) = 24; \
   else if ((n_) < 480) (nb_) = 35; \
   else if ((n_) < 584) (nb_) = 36; \
   else if ((n_) < 667) (nb_) = 48; \
   else if ((n_) < 854) (nb_) = 51; \
   else if ((n_) < 1103) (nb_) = 48; \
   else if ((n_) < 1269) (nb_) = 72; \
   else if ((n_) < 1684) (nb_) = 83; \
   else if ((n_) < 2348) (nb_) = 72; \
   else (nb_) = 131; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
