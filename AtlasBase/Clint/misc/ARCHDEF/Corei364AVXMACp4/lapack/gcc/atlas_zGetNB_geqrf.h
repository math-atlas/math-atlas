#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,86,148,209,271,394,518,765,1012,2000
 * N : 25,86,148,209,271,394,518,765,1012,2000
 * NB : 4,12,12,24,24,36,36,48,64,108
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 55) (nb_) = 4; \
   else if ((n_) < 178) (nb_) = 12; \
   else if ((n_) < 332) (nb_) = 24; \
   else if ((n_) < 641) (nb_) = 36; \
   else if ((n_) < 888) (nb_) = 48; \
   else if ((n_) < 1506) (nb_) = 64; \
   else (nb_) = 108; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
