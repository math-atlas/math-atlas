#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,271,394,518,1012,1259,1506,2000
 * N : 25,271,394,518,1012,1259,1506,2000
 * NB : 24,24,24,48,48,60,168,168
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 456) (nb_) = 24; \
   else if ((n_) < 1135) (nb_) = 48; \
   else if ((n_) < 1382) (nb_) = 60; \
   else (nb_) = 168; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
