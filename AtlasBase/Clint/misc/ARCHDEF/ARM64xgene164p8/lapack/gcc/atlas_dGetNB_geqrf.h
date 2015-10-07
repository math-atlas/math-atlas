#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,86,148,209,271,518,1012,2000
 * N : 25,86,148,209,271,518,1012,2000
 * NB : 8,8,16,20,36,36,48,76
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 117) (nb_) = 8; \
   else if ((n_) < 178) (nb_) = 16; \
   else if ((n_) < 240) (nb_) = 20; \
   else if ((n_) < 765) (nb_) = 36; \
   else if ((n_) < 1506) (nb_) = 48; \
   else (nb_) = 76; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
