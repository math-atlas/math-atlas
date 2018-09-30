#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,102,180,219,258,297,336,375,414,453,492,531,570,609,648,959,1271,1582,1894,2206,2518,3765,5012,7506,10000
 * N : 25,102,180,219,258,297,336,375,414,453,492,531,570,609,648,959,1271,1582,1894,2206,2518,3765,5012,7506,10000
 * NB : 1,11,15,15,16,18,19,43,67,66,75,78,79,82,83,95,99,99,107,139,195,219,227,225,864
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 63) (nb_) = 1; \
   else if ((n_) < 141) (nb_) = 11; \
   else if ((n_) < 238) (nb_) = 15; \
   else if ((n_) < 277) (nb_) = 16; \
   else if ((n_) < 316) (nb_) = 18; \
   else if ((n_) < 355) (nb_) = 19; \
   else if ((n_) < 394) (nb_) = 43; \
   else if ((n_) < 433) (nb_) = 67; \
   else if ((n_) < 472) (nb_) = 66; \
   else if ((n_) < 511) (nb_) = 75; \
   else if ((n_) < 550) (nb_) = 78; \
   else if ((n_) < 589) (nb_) = 79; \
   else if ((n_) < 628) (nb_) = 82; \
   else if ((n_) < 803) (nb_) = 83; \
   else if ((n_) < 1115) (nb_) = 95; \
   else if ((n_) < 1738) (nb_) = 99; \
   else if ((n_) < 2050) (nb_) = 107; \
   else if ((n_) < 2362) (nb_) = 139; \
   else if ((n_) < 3141) (nb_) = 195; \
   else if ((n_) < 4388) (nb_) = 219; \
   else if ((n_) < 6259) (nb_) = 227; \
   else if ((n_) < 8753) (nb_) = 225; \
   else (nb_) = 864; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
