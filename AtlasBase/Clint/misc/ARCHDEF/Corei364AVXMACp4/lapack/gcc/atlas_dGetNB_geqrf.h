#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,148,209,271,394,518,765,1012,1506,2000
 * N : 25,148,209,271,394,518,765,1012,1506,2000
 * NB : 12,12,24,24,36,36,48,100,120,168
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 178) (nb_) = 12; \
   else if ((n_) < 332) (nb_) = 24; \
   else if ((n_) < 641) (nb_) = 36; \
   else if ((n_) < 888) (nb_) = 48; \
   else if ((n_) < 1259) (nb_) = 100; \
   else if ((n_) < 1753) (nb_) = 120; \
   else (nb_) = 168; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
