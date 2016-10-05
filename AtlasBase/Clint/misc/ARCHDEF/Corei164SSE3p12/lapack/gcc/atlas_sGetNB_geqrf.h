#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,86,148,209,271,518,1012,2000
 * N : 25,86,148,209,271,518,1012,2000
 * NB : 12,12,16,20,20,36,68,108
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 117) (nb_) = 12; \
   else if ((n_) < 178) (nb_) = 16; \
   else if ((n_) < 394) (nb_) = 20; \
   else if ((n_) < 765) (nb_) = 36; \
   else if ((n_) < 1506) (nb_) = 68; \
   else (nb_) = 108; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
