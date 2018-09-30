#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,64,103,142,182,261,340,498,577,656,814,972,1130,1288,1920,2552,3816,5080
 * N : 25,64,103,142,182,261,340,498,577,656,814,972,1130,1288,1920,2552,3816,5080
 * NB : 1,1,15,12,27,30,43,42,50,51,48,60,108,131,147,192,216,384
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 83) (nb_) = 1; \
   else if ((n_) < 122) (nb_) = 15; \
   else if ((n_) < 162) (nb_) = 12; \
   else if ((n_) < 221) (nb_) = 27; \
   else if ((n_) < 300) (nb_) = 30; \
   else if ((n_) < 419) (nb_) = 43; \
   else if ((n_) < 537) (nb_) = 42; \
   else if ((n_) < 616) (nb_) = 50; \
   else if ((n_) < 735) (nb_) = 51; \
   else if ((n_) < 893) (nb_) = 48; \
   else if ((n_) < 1051) (nb_) = 60; \
   else if ((n_) < 1209) (nb_) = 108; \
   else if ((n_) < 1604) (nb_) = 131; \
   else if ((n_) < 2236) (nb_) = 147; \
   else if ((n_) < 3184) (nb_) = 192; \
   else if ((n_) < 4448) (nb_) = 216; \
   else (nb_) = 384; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
