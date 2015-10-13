#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,86,148,271,518,765,826,888,950,1012,2000
 * N : 25,86,148,271,518,765,826,888,950,1012,2000
 * NB : 4,8,12,16,24,24,48,48,48,68,116
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 55) (nb_) = 4; \
   else if ((n_) < 117) (nb_) = 8; \
   else if ((n_) < 209) (nb_) = 12; \
   else if ((n_) < 394) (nb_) = 16; \
   else if ((n_) < 795) (nb_) = 24; \
   else if ((n_) < 981) (nb_) = 48; \
   else if ((n_) < 1506) (nb_) = 68; \
   else (nb_) = 116; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
