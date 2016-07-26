#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,216,288,360,504,576,648,720,792,1080,1368,1440,1512,1656,1728,1800,1944,2088,2160,2232,4464,9000
 * N : 25,216,288,360,504,576,648,720,792,1080,1368,1440,1512,1656,1728,1800,1944,2088,2160,2232,4464,9000
 * NB : 5,12,24,24,24,24,88,24,136,84,72,72,96,96,84,132,120,72,72,132,120,96
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 120) (nb_) = 5; \
   else if ((n_) < 252) (nb_) = 12; \
   else if ((n_) < 612) (nb_) = 24; \
   else if ((n_) < 684) (nb_) = 88; \
   else if ((n_) < 756) (nb_) = 24; \
   else if ((n_) < 936) (nb_) = 136; \
   else if ((n_) < 1224) (nb_) = 84; \
   else if ((n_) < 1476) (nb_) = 72; \
   else if ((n_) < 1692) (nb_) = 96; \
   else if ((n_) < 1764) (nb_) = 84; \
   else if ((n_) < 1872) (nb_) = 132; \
   else if ((n_) < 2016) (nb_) = 120; \
   else if ((n_) < 2196) (nb_) = 72; \
   else if ((n_) < 3348) (nb_) = 132; \
   else if ((n_) < 6732) (nb_) = 120; \
   else (nb_) = 96; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
