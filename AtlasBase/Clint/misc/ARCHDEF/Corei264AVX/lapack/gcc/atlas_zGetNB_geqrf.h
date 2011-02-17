#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,180,360,540,660,780,960,1080,1140,1200,1380,1500,1560,1620
 * N : 25,180,360,540,660,780,960,1080,1140,1200,1380,1500,1560,1620
 * NB : 8,60,60,60,60,72,60,60,60,120,120,70,60,220
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 102) (nb_) = 8; \
   else if ((n_) < 720) (nb_) = 60; \
   else if ((n_) < 870) (nb_) = 72; \
   else if ((n_) < 1170) (nb_) = 60; \
   else if ((n_) < 1440) (nb_) = 120; \
   else if ((n_) < 1530) (nb_) = 70; \
   else if ((n_) < 1590) (nb_) = 60; \
   else (nb_) = 220; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
