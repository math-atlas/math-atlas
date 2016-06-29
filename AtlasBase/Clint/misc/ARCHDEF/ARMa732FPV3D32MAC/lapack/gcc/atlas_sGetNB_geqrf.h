#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,240,480,576,624,672,720,960,1440,1968
 * N : 25,240,480,576,624,672,720,960,1440,1968
 * NB : 12,12,20,16,24,20,48,48,96,96
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 360) (nb_) = 12; \
   else if ((n_) < 528) (nb_) = 20; \
   else if ((n_) < 600) (nb_) = 16; \
   else if ((n_) < 648) (nb_) = 24; \
   else if ((n_) < 696) (nb_) = 20; \
   else if ((n_) < 1200) (nb_) = 48; \
   else (nb_) = 96; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
