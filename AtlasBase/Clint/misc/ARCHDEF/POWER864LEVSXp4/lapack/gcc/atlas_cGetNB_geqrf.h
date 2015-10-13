#ifndef ATL_cGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,86,148,209,271,394,518,1012,2000
 * N : 25,86,148,209,271,394,518,1012,2000
 * NB : 4,16,16,24,32,32,40,72,108
 */
#define ATL_cGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 55) (nb_) = 4; \
   else if ((n_) < 178) (nb_) = 16; \
   else if ((n_) < 240) (nb_) = 24; \
   else if ((n_) < 456) (nb_) = 32; \
   else if ((n_) < 765) (nb_) = 40; \
   else if ((n_) < 1506) (nb_) = 72; \
   else (nb_) = 108; \
}


#endif    /* end ifndef ATL_cGetNB_geqrf */
