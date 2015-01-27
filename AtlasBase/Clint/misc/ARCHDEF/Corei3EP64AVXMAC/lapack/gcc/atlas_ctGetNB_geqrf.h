#ifndef ATL_ctGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,87,149,211,273,335,397,521,645,707,769,1018,2012,4000
 * N : 25,87,149,211,273,335,397,521,645,707,769,1018,2012,4000
 * NB : 4,16,16,24,28,28,60,60,60,60,100,100,100,116
 */
#define ATL_ctGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 56) (nb_) = 4; \
   else if ((n_) < 180) (nb_) = 16; \
   else if ((n_) < 242) (nb_) = 24; \
   else if ((n_) < 366) (nb_) = 28; \
   else if ((n_) < 738) (nb_) = 60; \
   else if ((n_) < 3006) (nb_) = 100; \
   else (nb_) = 116; \
}


#endif    /* end ifndef ATL_ctGetNB_geqrf */
