#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,88,151,214,277,340,403,466,530,656,783,909,1036,1289,1542,1795,2048,2554,3060,4072,8120
 * N : 25,88,151,214,277,340,403,466,530,656,783,909,1036,1289,1542,1795,2048,2554,3060,4072,8120
 * NB : 1,1,11,14,15,16,18,18,19,19,43,59,67,68,71,71,72,288,288,288,288
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 119) (nb_) = 1; \
   else if ((n_) < 182) (nb_) = 11; \
   else if ((n_) < 245) (nb_) = 14; \
   else if ((n_) < 308) (nb_) = 15; \
   else if ((n_) < 371) (nb_) = 16; \
   else if ((n_) < 498) (nb_) = 18; \
   else if ((n_) < 719) (nb_) = 19; \
   else if ((n_) < 846) (nb_) = 43; \
   else if ((n_) < 972) (nb_) = 59; \
   else if ((n_) < 1162) (nb_) = 67; \
   else if ((n_) < 1415) (nb_) = 68; \
   else if ((n_) < 1921) (nb_) = 71; \
   else if ((n_) < 2301) (nb_) = 72; \
   else (nb_) = 288; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
