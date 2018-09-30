#ifndef ATL_cGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,85,145,205,266,326,387,447,508,629,750,871,992,1234,1476,1718,1960
 * N : 25,85,145,205,266,326,387,447,508,629,750,871,992,1234,1476,1718,1960
 * NB : 1,1,15,16,19,19,20,20,23,23,24,30,31,31,32,32,35
 */
#define ATL_cGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 115) (nb_) = 1; \
   else if ((n_) < 175) (nb_) = 15; \
   else if ((n_) < 235) (nb_) = 16; \
   else if ((n_) < 356) (nb_) = 19; \
   else if ((n_) < 477) (nb_) = 20; \
   else if ((n_) < 689) (nb_) = 23; \
   else if ((n_) < 810) (nb_) = 24; \
   else if ((n_) < 931) (nb_) = 30; \
   else if ((n_) < 1355) (nb_) = 31; \
   else if ((n_) < 1839) (nb_) = 32; \
   else (nb_) = 35; \
}


#endif    /* end ifndef ATL_cGetNB_geqrf */
