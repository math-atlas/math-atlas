#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,102,180,218,257,296,335,490,646,723,801,879,957,1112,1268,1890,2512,3756,5000
 * N : 25,102,180,218,257,296,335,490,646,723,801,879,957,1112,1268,1890,2512,3756,5000
 * NB : 1,1,7,15,23,24,27,31,43,43,51,52,55,55,67,64,83,95,99
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 141) (nb_) = 1; \
   else if ((n_) < 199) (nb_) = 7; \
   else if ((n_) < 237) (nb_) = 15; \
   else if ((n_) < 276) (nb_) = 23; \
   else if ((n_) < 315) (nb_) = 24; \
   else if ((n_) < 412) (nb_) = 27; \
   else if ((n_) < 568) (nb_) = 31; \
   else if ((n_) < 762) (nb_) = 43; \
   else if ((n_) < 840) (nb_) = 51; \
   else if ((n_) < 918) (nb_) = 52; \
   else if ((n_) < 1190) (nb_) = 55; \
   else if ((n_) < 1579) (nb_) = 67; \
   else if ((n_) < 2201) (nb_) = 64; \
   else if ((n_) < 3134) (nb_) = 83; \
   else if ((n_) < 4378) (nb_) = 95; \
   else (nb_) = 99; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
