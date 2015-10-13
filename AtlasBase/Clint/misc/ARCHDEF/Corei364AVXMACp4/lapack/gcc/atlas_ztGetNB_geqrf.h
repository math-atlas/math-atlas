#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,87,149,211,273,521,1018,1515,1763,2012,3006,3503,3751,3875,3937,4000
 * N : 25,87,149,211,273,521,1018,1515,1763,2012,3006,3503,3751,3875,3937,4000
 * NB : 8,12,28,28,32,36,48,48,64,84,108,108,108,108,140,172
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 56) (nb_) = 8; \
   else if ((n_) < 118) (nb_) = 12; \
   else if ((n_) < 242) (nb_) = 28; \
   else if ((n_) < 397) (nb_) = 32; \
   else if ((n_) < 769) (nb_) = 36; \
   else if ((n_) < 1639) (nb_) = 48; \
   else if ((n_) < 1887) (nb_) = 64; \
   else if ((n_) < 2509) (nb_) = 84; \
   else if ((n_) < 3906) (nb_) = 108; \
   else if ((n_) < 3968) (nb_) = 140; \
   else (nb_) = 172; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
