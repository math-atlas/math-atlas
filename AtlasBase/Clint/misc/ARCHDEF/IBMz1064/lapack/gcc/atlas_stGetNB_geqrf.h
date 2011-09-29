#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,216,360,504,1080,1152,1296,1440,1584,2160,3240,4392
 * N : 25,216,360,504,1080,1152,1296,1440,1584,2160,3240,4392
 * NB : 4,12,12,28,16,24,24,72,72,72,72,112
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 120) (nb_) = 4; \
   else if ((n_) < 432) (nb_) = 12; \
   else if ((n_) < 792) (nb_) = 28; \
   else if ((n_) < 1116) (nb_) = 16; \
   else if ((n_) < 1368) (nb_) = 24; \
   else if ((n_) < 3816) (nb_) = 72; \
   else (nb_) = 112; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
