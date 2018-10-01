#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,165,306,447,588,870,1152,1187,1222,1257,1293,1328,1363,1398,1434,1504,1575,1645,1716,1998,2280
 * N : 25,165,306,447,588,870,1152,1187,1222,1257,1293,1328,1363,1398,1434,1504,1575,1645,1716,1998,2280
 * NB : 1,1,4,4,6,6,7,11,13,14,15,17,21,21,23,23,24,24,27,27,35
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 235) (nb_) = 1; \
   else if ((n_) < 517) (nb_) = 4; \
   else if ((n_) < 1011) (nb_) = 6; \
   else if ((n_) < 1169) (nb_) = 7; \
   else if ((n_) < 1204) (nb_) = 11; \
   else if ((n_) < 1239) (nb_) = 13; \
   else if ((n_) < 1275) (nb_) = 14; \
   else if ((n_) < 1310) (nb_) = 15; \
   else if ((n_) < 1345) (nb_) = 17; \
   else if ((n_) < 1416) (nb_) = 21; \
   else if ((n_) < 1539) (nb_) = 23; \
   else if ((n_) < 1680) (nb_) = 24; \
   else if ((n_) < 2139) (nb_) = 27; \
   else (nb_) = 35; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
