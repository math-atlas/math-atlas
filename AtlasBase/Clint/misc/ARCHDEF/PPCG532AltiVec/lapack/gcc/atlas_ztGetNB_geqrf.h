#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,96,144,192,240,480,576,624,672,720,816,864,960,1920
 * N : 25,96,144,192,240,480,576,624,672,720,816,864,960,1920
 * NB : 4,12,12,48,48,48,48,8,36,56,48,128,96,96
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 60) (nb_) = 4; \
   else if ((n_) < 168) (nb_) = 12; \
   else if ((n_) < 600) (nb_) = 48; \
   else if ((n_) < 648) (nb_) = 8; \
   else if ((n_) < 696) (nb_) = 36; \
   else if ((n_) < 768) (nb_) = 56; \
   else if ((n_) < 840) (nb_) = 48; \
   else if ((n_) < 912) (nb_) = 128; \
   else (nb_) = 96; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
