#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,149,211,273,335,397,521,583,645,769,1018,1515,1763,2012,4000
 * N : 25,149,211,273,335,397,521,583,645,769,1018,1515,1763,2012,4000
 * NB : 12,12,16,40,44,52,52,68,68,68,68,108,108,132,180
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 180) (nb_) = 12; \
   else if ((n_) < 242) (nb_) = 16; \
   else if ((n_) < 304) (nb_) = 40; \
   else if ((n_) < 366) (nb_) = 44; \
   else if ((n_) < 552) (nb_) = 52; \
   else if ((n_) < 1266) (nb_) = 68; \
   else if ((n_) < 1887) (nb_) = 108; \
   else if ((n_) < 3006) (nb_) = 132; \
   else (nb_) = 180; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
