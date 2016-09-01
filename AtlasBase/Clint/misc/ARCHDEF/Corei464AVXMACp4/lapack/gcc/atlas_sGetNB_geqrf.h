#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,86,148,271,394,518,765,1012,1259,1506,2000
 * N : 25,86,148,271,394,518,765,1012,1259,1506,2000
 * NB : 12,12,24,24,28,40,68,68,92,156,156
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 117) (nb_) = 12; \
   else if ((n_) < 332) (nb_) = 24; \
   else if ((n_) < 456) (nb_) = 28; \
   else if ((n_) < 641) (nb_) = 40; \
   else if ((n_) < 1135) (nb_) = 68; \
   else if ((n_) < 1382) (nb_) = 92; \
   else (nb_) = 156; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
