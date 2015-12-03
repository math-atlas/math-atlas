#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,148,209,271,394,518,1012,2000
 * N : 25,148,209,271,394,518,1012,2000
 * NB : 12,12,12,28,36,48,64,100
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 240) (nb_) = 12; \
   else if ((n_) < 332) (nb_) = 28; \
   else if ((n_) < 456) (nb_) = 36; \
   else if ((n_) < 765) (nb_) = 48; \
   else if ((n_) < 1506) (nb_) = 64; \
   else (nb_) = 100; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
