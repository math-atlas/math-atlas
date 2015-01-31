#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,271,394,518,765,1012,1506,2000
 * N : 25,271,394,518,765,1012,1506,2000
 * NB : 24,24,24,48,48,60,96,108
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 456) (nb_) = 24; \
   else if ((n_) < 888) (nb_) = 48; \
   else if ((n_) < 1259) (nb_) = 60; \
   else if ((n_) < 1753) (nb_) = 96; \
   else (nb_) = 108; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
