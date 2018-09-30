#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,130,236,289,342,395,448,660,872,925,978,1031,1084,1190,1296,1508,1720
 * N : 25,130,236,289,342,395,448,660,872,925,978,1031,1084,1190,1296,1508,1720
 * NB : 1,1,9,9,10,10,11,11,16,64,72,72,80,76,96,96,99
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 183) (nb_) = 1; \
   else if ((n_) < 315) (nb_) = 9; \
   else if ((n_) < 421) (nb_) = 10; \
   else if ((n_) < 766) (nb_) = 11; \
   else if ((n_) < 898) (nb_) = 16; \
   else if ((n_) < 951) (nb_) = 64; \
   else if ((n_) < 1057) (nb_) = 72; \
   else if ((n_) < 1137) (nb_) = 80; \
   else if ((n_) < 1243) (nb_) = 76; \
   else if ((n_) < 1614) (nb_) = 96; \
   else (nb_) = 99; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
