#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,120,240,360,480,600,660,720,960,1980
 * N : 25,120,240,360,480,600,660,720,960,1980
 * NB : 4,12,12,12,24,36,60,60,60,60
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 72) (nb_) = 4; \
   else if ((n_) < 420) (nb_) = 12; \
   else if ((n_) < 540) (nb_) = 24; \
   else if ((n_) < 630) (nb_) = 36; \
   else (nb_) = 60; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
