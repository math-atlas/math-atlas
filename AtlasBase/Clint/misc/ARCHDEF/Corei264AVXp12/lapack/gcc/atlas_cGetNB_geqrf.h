#ifndef ATL_cGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,86,148,271,332,394,518,765,1012,2000
 * N : 25,86,148,271,332,394,518,765,1012,2000
 * NB : 8,20,20,20,24,24,48,48,96,96
 */
#define ATL_cGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 55) (nb_) = 8; \
   else if ((n_) < 301) (nb_) = 20; \
   else if ((n_) < 456) (nb_) = 24; \
   else if ((n_) < 888) (nb_) = 48; \
   else (nb_) = 96; \
}


#endif    /* end ifndef ATL_cGetNB_geqrf */
