#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,86,148,271,332,394,456,518,641,765,888,1012,2000
 * N : 25,86,148,271,332,394,456,518,641,765,888,1012,2000
 * NB : 12,12,24,24,24,36,36,40,48,48,76,76,144
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 117) (nb_) = 12; \
   else if ((n_) < 363) (nb_) = 24; \
   else if ((n_) < 487) (nb_) = 36; \
   else if ((n_) < 579) (nb_) = 40; \
   else if ((n_) < 826) (nb_) = 48; \
   else if ((n_) < 1506) (nb_) = 76; \
   else (nb_) = 144; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
