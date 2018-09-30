#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,110,195,366,537,708,1050,1392,1434,1477,1520,1563,1605,1648,1691,1734,1905,2076,2418,2760
 * N : 25,110,195,366,537,708,1050,1392,1434,1477,1520,1563,1605,1648,1691,1734,1905,2076,2418,2760
 * NB : 1,1,2,2,2,4,4,7,11,15,16,19,19,20,20,23,24,27,29,35
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 152) (nb_) = 1; \
   else if ((n_) < 622) (nb_) = 2; \
   else if ((n_) < 1221) (nb_) = 4; \
   else if ((n_) < 1413) (nb_) = 7; \
   else if ((n_) < 1455) (nb_) = 11; \
   else if ((n_) < 1498) (nb_) = 15; \
   else if ((n_) < 1541) (nb_) = 16; \
   else if ((n_) < 1626) (nb_) = 19; \
   else if ((n_) < 1712) (nb_) = 20; \
   else if ((n_) < 1819) (nb_) = 23; \
   else if ((n_) < 1990) (nb_) = 24; \
   else if ((n_) < 2247) (nb_) = 27; \
   else if ((n_) < 2589) (nb_) = 29; \
   else (nb_) = 35; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
