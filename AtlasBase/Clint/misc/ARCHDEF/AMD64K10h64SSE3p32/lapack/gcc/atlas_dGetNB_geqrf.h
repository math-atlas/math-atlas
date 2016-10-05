#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,148,271,518,765,1012,2000
 * N : 25,148,271,518,765,1012,2000
 * NB : 12,12,24,40,40,84,124
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 209) (nb_) = 12; \
   else if ((n_) < 394) (nb_) = 24; \
   else if ((n_) < 888) (nb_) = 40; \
   else if ((n_) < 1506) (nb_) = 84; \
   else (nb_) = 124; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
