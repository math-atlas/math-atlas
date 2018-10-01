#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,62,100,176,252,328,366,404,442,480,556,632,708,784,860,936,1088,1240
 * N : 25,62,100,176,252,328,366,404,442,480,556,632,708,784,860,936,1088,1240
 * NB : 1,1,2,2,2,6,18,26,28,30,30,31,31,32,32,34,34,35
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 81) (nb_) = 1; \
   else if ((n_) < 290) (nb_) = 2; \
   else if ((n_) < 347) (nb_) = 6; \
   else if ((n_) < 385) (nb_) = 18; \
   else if ((n_) < 423) (nb_) = 26; \
   else if ((n_) < 461) (nb_) = 28; \
   else if ((n_) < 594) (nb_) = 30; \
   else if ((n_) < 746) (nb_) = 31; \
   else if ((n_) < 898) (nb_) = 32; \
   else if ((n_) < 1164) (nb_) = 34; \
   else (nb_) = 35; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
