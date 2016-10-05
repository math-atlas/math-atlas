#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,86,148,209,271,394,518,641,765,888,1012,2000
 * N : 25,86,148,209,271,394,518,641,765,888,1012,2000
 * NB : 12,12,16,24,24,24,44,60,72,72,92,144
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 117) (nb_) = 12; \
   else if ((n_) < 178) (nb_) = 16; \
   else if ((n_) < 456) (nb_) = 24; \
   else if ((n_) < 579) (nb_) = 44; \
   else if ((n_) < 703) (nb_) = 60; \
   else if ((n_) < 950) (nb_) = 72; \
   else if ((n_) < 1506) (nb_) = 92; \
   else (nb_) = 144; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
