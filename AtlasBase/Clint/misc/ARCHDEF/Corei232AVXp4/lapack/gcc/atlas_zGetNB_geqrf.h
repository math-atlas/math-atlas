#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,86,148,209,271,518,765,1012,2000
 * N : 25,86,148,209,271,518,765,1012,2000
 * NB : 8,8,12,20,20,24,48,76,100
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 117) (nb_) = 8; \
   else if ((n_) < 178) (nb_) = 12; \
   else if ((n_) < 394) (nb_) = 20; \
   else if ((n_) < 641) (nb_) = 24; \
   else if ((n_) < 888) (nb_) = 48; \
   else if ((n_) < 1506) (nb_) = 76; \
   else (nb_) = 100; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
