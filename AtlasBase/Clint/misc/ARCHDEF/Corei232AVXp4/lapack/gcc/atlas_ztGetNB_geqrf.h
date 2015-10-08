#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,87,149,211,273,521,1018,2012,2260,2509,3006,3254,3503,3751,4000
 * N : 25,87,149,211,273,521,1018,2012,2260,2509,3006,3254,3503,3751,4000
 * NB : 12,16,20,20,24,24,48,48,48,76,76,80,84,84,88
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 56) (nb_) = 12; \
   else if ((n_) < 118) (nb_) = 16; \
   else if ((n_) < 242) (nb_) = 20; \
   else if ((n_) < 769) (nb_) = 24; \
   else if ((n_) < 2384) (nb_) = 48; \
   else if ((n_) < 3130) (nb_) = 76; \
   else if ((n_) < 3378) (nb_) = 80; \
   else if ((n_) < 3875) (nb_) = 84; \
   else (nb_) = 88; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
