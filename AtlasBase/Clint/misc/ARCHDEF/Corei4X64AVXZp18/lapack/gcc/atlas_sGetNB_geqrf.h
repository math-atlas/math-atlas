#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,148,271,394,456,518,579,641,703,765,888,1012,1073,1135,1197,1259,1382,1506,1753,2000
 * N : 25,148,271,394,456,518,579,641,703,765,888,1012,1073,1135,1197,1259,1382,1506,1753,2000
 * NB : 1,1,15,14,34,35,55,59,71,75,81,83,87,89,90,91,94,95,96,99
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 209) (nb_) = 1; \
   else if ((n_) < 332) (nb_) = 15; \
   else if ((n_) < 425) (nb_) = 14; \
   else if ((n_) < 487) (nb_) = 34; \
   else if ((n_) < 548) (nb_) = 35; \
   else if ((n_) < 610) (nb_) = 55; \
   else if ((n_) < 672) (nb_) = 59; \
   else if ((n_) < 734) (nb_) = 71; \
   else if ((n_) < 826) (nb_) = 75; \
   else if ((n_) < 950) (nb_) = 81; \
   else if ((n_) < 1042) (nb_) = 83; \
   else if ((n_) < 1104) (nb_) = 87; \
   else if ((n_) < 1166) (nb_) = 89; \
   else if ((n_) < 1228) (nb_) = 90; \
   else if ((n_) < 1320) (nb_) = 91; \
   else if ((n_) < 1444) (nb_) = 94; \
   else if ((n_) < 1629) (nb_) = 95; \
   else if ((n_) < 1876) (nb_) = 96; \
   else (nb_) = 99; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
