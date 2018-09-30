#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,91,157,190,223,256,290,356,423,489,556,589,622,655,689,755,822,955,1088,1620,2152,3216,4280
 * N : 25,91,157,190,223,256,290,356,423,489,556,589,622,655,689,755,822,955,1088,1620,2152,3216,4280
 * NB : 1,1,7,19,23,24,27,27,28,28,35,35,36,36,39,39,40,40,83,123,131,128,163
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 124) (nb_) = 1; \
   else if ((n_) < 173) (nb_) = 7; \
   else if ((n_) < 206) (nb_) = 19; \
   else if ((n_) < 239) (nb_) = 23; \
   else if ((n_) < 273) (nb_) = 24; \
   else if ((n_) < 389) (nb_) = 27; \
   else if ((n_) < 522) (nb_) = 28; \
   else if ((n_) < 605) (nb_) = 35; \
   else if ((n_) < 672) (nb_) = 36; \
   else if ((n_) < 788) (nb_) = 39; \
   else if ((n_) < 1021) (nb_) = 40; \
   else if ((n_) < 1354) (nb_) = 83; \
   else if ((n_) < 1886) (nb_) = 123; \
   else if ((n_) < 2684) (nb_) = 131; \
   else if ((n_) < 3748) (nb_) = 128; \
   else (nb_) = 163; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
