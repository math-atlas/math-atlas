#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,271,518,765,826,888,1012,1506,2000
 * N : 25,271,518,765,826,888,1012,1506,2000
 * NB : 24,24,28,28,32,76,76,192,192
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 394) (nb_) = 24; \
   else if ((n_) < 795) (nb_) = 28; \
   else if ((n_) < 857) (nb_) = 32; \
   else if ((n_) < 1259) (nb_) = 76; \
   else (nb_) = 192; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
