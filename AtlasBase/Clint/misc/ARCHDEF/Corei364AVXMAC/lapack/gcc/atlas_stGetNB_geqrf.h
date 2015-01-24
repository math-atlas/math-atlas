#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,87,149,211,273,335,397,521,1018,1515,2012,2260,2322,2384,2509,3006,3254,3503,3751,4000
 * N : 25,87,149,211,273,335,397,521,1018,1515,2012,2260,2322,2384,2509,3006,3254,3503,3751,4000
 * NB : 12,12,16,24,24,24,68,68,68,68,92,96,112,124,124,128,128,148,204,204
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 118) (nb_) = 12; \
   else if ((n_) < 180) (nb_) = 16; \
   else if ((n_) < 366) (nb_) = 24; \
   else if ((n_) < 1763) (nb_) = 68; \
   else if ((n_) < 2136) (nb_) = 92; \
   else if ((n_) < 2291) (nb_) = 96; \
   else if ((n_) < 2353) (nb_) = 112; \
   else if ((n_) < 2757) (nb_) = 124; \
   else if ((n_) < 3378) (nb_) = 128; \
   else if ((n_) < 3627) (nb_) = 148; \
   else (nb_) = 204; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
