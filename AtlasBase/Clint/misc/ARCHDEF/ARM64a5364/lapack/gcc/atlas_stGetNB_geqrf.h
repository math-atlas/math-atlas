#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,96,144,192,288,336,384,432,528,576,624,720,768,816,864,1728,3456
 * N : 25,96,144,192,288,336,384,432,528,576,624,720,768,816,864,1728,3456
 * NB : 2,8,8,16,12,16,48,48,48,48,60,48,48,48,96,96,192
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 60) (nb_) = 2; \
   else if ((n_) < 168) (nb_) = 8; \
   else if ((n_) < 240) (nb_) = 16; \
   else if ((n_) < 312) (nb_) = 12; \
   else if ((n_) < 360) (nb_) = 16; \
   else if ((n_) < 600) (nb_) = 48; \
   else if ((n_) < 672) (nb_) = 60; \
   else if ((n_) < 840) (nb_) = 48; \
   else if ((n_) < 2592) (nb_) = 96; \
   else (nb_) = 192; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
