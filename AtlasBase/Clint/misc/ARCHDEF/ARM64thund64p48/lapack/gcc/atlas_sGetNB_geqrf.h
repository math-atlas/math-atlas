#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,648,1272,1311,1350,1389,1428,1506,1584,1662,1740,1818,1896,1974,2052,2130,2208,2364,2520
 * N : 25,648,1272,1311,1350,1389,1428,1506,1584,1662,1740,1818,1896,1974,2052,2130,2208,2364,2520
 * NB : 1,1,7,7,8,9,10,10,11,11,12,12,15,15,16,17,18,18,19
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 960) (nb_) = 1; \
   else if ((n_) < 1330) (nb_) = 7; \
   else if ((n_) < 1369) (nb_) = 8; \
   else if ((n_) < 1408) (nb_) = 9; \
   else if ((n_) < 1545) (nb_) = 10; \
   else if ((n_) < 1701) (nb_) = 11; \
   else if ((n_) < 1857) (nb_) = 12; \
   else if ((n_) < 2013) (nb_) = 15; \
   else if ((n_) < 2091) (nb_) = 16; \
   else if ((n_) < 2169) (nb_) = 17; \
   else if ((n_) < 2442) (nb_) = 18; \
   else (nb_) = 19; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
