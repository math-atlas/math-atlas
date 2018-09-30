#ifndef ATL_cGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,148,271,332,394,456,518,641,765,826,888,950,1012,1506,2000
 * N : 25,148,271,332,394,456,518,641,765,826,888,950,1012,1506,2000
 * NB : 1,1,35,30,47,48,51,54,55,55,95,95,112,112,384
 */
#define ATL_cGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 209) (nb_) = 1; \
   else if ((n_) < 301) (nb_) = 35; \
   else if ((n_) < 363) (nb_) = 30; \
   else if ((n_) < 425) (nb_) = 47; \
   else if ((n_) < 487) (nb_) = 48; \
   else if ((n_) < 579) (nb_) = 51; \
   else if ((n_) < 703) (nb_) = 54; \
   else if ((n_) < 857) (nb_) = 55; \
   else if ((n_) < 981) (nb_) = 95; \
   else if ((n_) < 1753) (nb_) = 112; \
   else (nb_) = 384; \
}


#endif    /* end ifndef ATL_cGetNB_geqrf */
