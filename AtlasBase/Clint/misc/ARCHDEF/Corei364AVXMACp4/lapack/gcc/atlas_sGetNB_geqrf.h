#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,86,148,209,271,518,579,641,765,1012,1506,2000
 * N : 25,86,148,209,271,518,579,641,765,1012,1506,2000
 * NB : 8,8,16,24,24,28,40,96,96,96,96,172
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 117) (nb_) = 8; \
   else if ((n_) < 178) (nb_) = 16; \
   else if ((n_) < 394) (nb_) = 24; \
   else if ((n_) < 548) (nb_) = 28; \
   else if ((n_) < 610) (nb_) = 40; \
   else if ((n_) < 1753) (nb_) = 96; \
   else (nb_) = 172; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
