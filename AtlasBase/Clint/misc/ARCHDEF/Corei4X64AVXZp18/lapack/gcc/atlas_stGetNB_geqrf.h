#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,87,149,211,273,335,397,459,521,583,645,707,769,831,893,955,1018,1515,2012,2509,3006,3254,3503,4000
 * N : 25,87,149,211,273,335,397,459,521,583,645,707,769,831,893,955,1018,1515,2012,2509,3006,3254,3503,4000
 * NB : 1,3,6,6,7,27,31,33,35,35,37,37,43,44,45,46,51,67,128,128,129,145,256,256
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 56) (nb_) = 1; \
   else if ((n_) < 118) (nb_) = 3; \
   else if ((n_) < 242) (nb_) = 6; \
   else if ((n_) < 304) (nb_) = 7; \
   else if ((n_) < 366) (nb_) = 27; \
   else if ((n_) < 428) (nb_) = 31; \
   else if ((n_) < 490) (nb_) = 33; \
   else if ((n_) < 614) (nb_) = 35; \
   else if ((n_) < 738) (nb_) = 37; \
   else if ((n_) < 800) (nb_) = 43; \
   else if ((n_) < 862) (nb_) = 44; \
   else if ((n_) < 924) (nb_) = 45; \
   else if ((n_) < 986) (nb_) = 46; \
   else if ((n_) < 1266) (nb_) = 51; \
   else if ((n_) < 1763) (nb_) = 67; \
   else if ((n_) < 2757) (nb_) = 128; \
   else if ((n_) < 3130) (nb_) = 129; \
   else if ((n_) < 3378) (nb_) = 145; \
   else (nb_) = 256; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
