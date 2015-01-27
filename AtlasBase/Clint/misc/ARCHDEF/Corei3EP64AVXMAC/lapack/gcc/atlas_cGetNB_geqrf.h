#ifndef ATL_cGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,86,148,209,271,518,579,641,765,1012,1506,2000
 * N : 25,86,148,209,271,518,579,641,765,1012,1506,2000
 * NB : 4,8,16,24,24,24,28,60,60,60,100,100
 */
#define ATL_cGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 55) (nb_) = 4; \
   else if ((n_) < 117) (nb_) = 8; \
   else if ((n_) < 178) (nb_) = 16; \
   else if ((n_) < 548) (nb_) = 24; \
   else if ((n_) < 610) (nb_) = 28; \
   else if ((n_) < 1259) (nb_) = 60; \
   else (nb_) = 100; \
}


#endif    /* end ifndef ATL_cGetNB_geqrf */
