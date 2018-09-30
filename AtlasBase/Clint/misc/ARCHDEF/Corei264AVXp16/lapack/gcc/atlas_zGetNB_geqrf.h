#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,648,1272,1311,1350,1389,1428,1467,1506,1545,1584,1662,1740,1818,1896,2208,2520
 * N : 25,648,1272,1311,1350,1389,1428,1467,1506,1545,1584,1662,1740,1818,1896,2208,2520
 * NB : 1,1,7,11,13,14,15,15,21,21,23,23,24,24,27,27,33
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 960) (nb_) = 1; \
   else if ((n_) < 1291) (nb_) = 7; \
   else if ((n_) < 1330) (nb_) = 11; \
   else if ((n_) < 1369) (nb_) = 13; \
   else if ((n_) < 1408) (nb_) = 14; \
   else if ((n_) < 1486) (nb_) = 15; \
   else if ((n_) < 1564) (nb_) = 21; \
   else if ((n_) < 1701) (nb_) = 23; \
   else if ((n_) < 1857) (nb_) = 24; \
   else if ((n_) < 2364) (nb_) = 27; \
   else (nb_) = 33; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
