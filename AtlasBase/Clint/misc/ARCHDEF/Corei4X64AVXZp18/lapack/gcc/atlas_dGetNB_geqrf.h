#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,86,148,209,271,332,394,456,518,641,765,888,1012,1506,2000
 * N : 25,86,148,209,271,332,394,456,518,641,765,888,1012,1506,2000
 * NB : 1,1,7,27,35,43,59,60,83,84,95,96,99,90,163
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 117) (nb_) = 1; \
   else if ((n_) < 178) (nb_) = 7; \
   else if ((n_) < 240) (nb_) = 27; \
   else if ((n_) < 301) (nb_) = 35; \
   else if ((n_) < 363) (nb_) = 43; \
   else if ((n_) < 425) (nb_) = 59; \
   else if ((n_) < 487) (nb_) = 60; \
   else if ((n_) < 579) (nb_) = 83; \
   else if ((n_) < 703) (nb_) = 84; \
   else if ((n_) < 826) (nb_) = 95; \
   else if ((n_) < 950) (nb_) = 96; \
   else if ((n_) < 1259) (nb_) = 99; \
   else if ((n_) < 1753) (nb_) = 90; \
   else (nb_) = 163; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
