#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,107,190,231,273,314,356,522,688,854,937,1020,1186,1352,2016,2680
 * N : 25,107,190,231,273,314,356,522,688,854,937,1020,1186,1352,2016,2680
 * NB : 1,1,23,23,24,24,27,28,35,32,52,59,71,83,72,131
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 148) (nb_) = 1; \
   else if ((n_) < 252) (nb_) = 23; \
   else if ((n_) < 335) (nb_) = 24; \
   else if ((n_) < 439) (nb_) = 27; \
   else if ((n_) < 605) (nb_) = 28; \
   else if ((n_) < 771) (nb_) = 35; \
   else if ((n_) < 895) (nb_) = 32; \
   else if ((n_) < 978) (nb_) = 52; \
   else if ((n_) < 1103) (nb_) = 59; \
   else if ((n_) < 1269) (nb_) = 71; \
   else if ((n_) < 1684) (nb_) = 83; \
   else if ((n_) < 2348) (nb_) = 72; \
   else (nb_) = 131; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
