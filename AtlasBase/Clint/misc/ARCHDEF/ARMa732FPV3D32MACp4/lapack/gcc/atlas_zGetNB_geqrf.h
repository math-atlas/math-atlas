#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,148,209,271,394,518,1012,2000
 * N : 25,148,209,271,394,518,1012,2000
 * NB : 10,10,20,20,20,40,40,40
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 178) (nb_) = 10; \
   else if ((n_) < 456) (nb_) = 20; \
   else (nb_) = 40; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
