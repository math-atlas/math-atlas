#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,86,148,271,332,394,518,1012,2000
 * N : 25,86,148,271,332,394,518,1012,2000
 * NB : 4,16,16,16,28,28,40,68,128
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 55) (nb_) = 4; \
   else if ((n_) < 301) (nb_) = 16; \
   else if ((n_) < 456) (nb_) = 28; \
   else if ((n_) < 765) (nb_) = 40; \
   else if ((n_) < 1506) (nb_) = 68; \
   else (nb_) = 128; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
