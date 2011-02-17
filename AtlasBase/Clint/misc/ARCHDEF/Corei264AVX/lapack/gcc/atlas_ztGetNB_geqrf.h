#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,120,300,420,480,540,600,1260,2580
 * N : 25,120,300,420,480,540,600,1260,2580
 * NB : 9,60,60,12,32,20,88,60,60
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 72) (nb_) = 9; \
   else if ((n_) < 360) (nb_) = 60; \
   else if ((n_) < 450) (nb_) = 12; \
   else if ((n_) < 510) (nb_) = 32; \
   else if ((n_) < 570) (nb_) = 20; \
   else if ((n_) < 930) (nb_) = 88; \
   else (nb_) = 60; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
