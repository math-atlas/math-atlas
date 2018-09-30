#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,317,610,903,1196,1489,1782,2075,2368,2954,3540,4126,4712,7056,9400
 * N : 25,317,610,903,1196,1489,1782,2075,2368,2954,3540,4126,4712,7056,9400
 * NB : 1,1,23,23,35,43,44,60,131,136,259,263,264,296,480
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 463) (nb_) = 1; \
   else if ((n_) < 1049) (nb_) = 23; \
   else if ((n_) < 1342) (nb_) = 35; \
   else if ((n_) < 1635) (nb_) = 43; \
   else if ((n_) < 1928) (nb_) = 44; \
   else if ((n_) < 2221) (nb_) = 60; \
   else if ((n_) < 2661) (nb_) = 131; \
   else if ((n_) < 3247) (nb_) = 136; \
   else if ((n_) < 3833) (nb_) = 259; \
   else if ((n_) < 4419) (nb_) = 263; \
   else if ((n_) < 5884) (nb_) = 264; \
   else if ((n_) < 8228) (nb_) = 296; \
   else (nb_) = 480; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
