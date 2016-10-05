#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,86,148,271,394,518,1012,2000
 * N : 25,86,148,271,394,518,1012,2000
 * NB : 6,12,12,16,20,36,52,100
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 55) (nb_) = 6; \
   else if ((n_) < 209) (nb_) = 12; \
   else if ((n_) < 332) (nb_) = 16; \
   else if ((n_) < 456) (nb_) = 20; \
   else if ((n_) < 765) (nb_) = 36; \
   else if ((n_) < 1506) (nb_) = 52; \
   else (nb_) = 100; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
