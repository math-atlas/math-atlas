#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,86,148,271,394,456,518,765,1012,2000
 * N : 25,86,148,271,394,456,518,765,1012,2000
 * NB : 12,12,24,24,24,24,48,48,96,96
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 117) (nb_) = 12; \
   else if ((n_) < 487) (nb_) = 24; \
   else if ((n_) < 888) (nb_) = 48; \
   else (nb_) = 96; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
