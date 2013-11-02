#ifndef ATL_ctGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,92,160,295,362,430,498,566,1108,1379,1650,1785,1921,2056,2192,2734,3276,4360
 * N : 25,92,160,295,362,430,498,566,1108,1379,1650,1785,1921,2056,2192,2734,3276,4360
 * NB : 12,24,32,40,40,44,72,72,72,72,76,96,96,96,104,112,144,160
 */
#define ATL_ctGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 58) (nb_) = 12; \
   else if ((n_) < 126) (nb_) = 24; \
   else if ((n_) < 227) (nb_) = 32; \
   else if ((n_) < 396) (nb_) = 40; \
   else if ((n_) < 464) (nb_) = 44; \
   else if ((n_) < 1514) (nb_) = 72; \
   else if ((n_) < 1717) (nb_) = 76; \
   else if ((n_) < 2124) (nb_) = 96; \
   else if ((n_) < 2463) (nb_) = 104; \
   else if ((n_) < 3005) (nb_) = 112; \
   else if ((n_) < 3818) (nb_) = 144; \
   else (nb_) = 160; \
}


#endif    /* end ifndef ATL_ctGetNB_geqrf */
