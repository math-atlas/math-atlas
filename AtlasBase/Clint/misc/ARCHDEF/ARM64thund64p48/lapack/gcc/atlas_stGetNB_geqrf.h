#ifndef ATL_stGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,88,152,184,216,248,280,343,407,471,535,790,1046,1557,2068,3090,4112,6156,8200
 * N : 25,88,152,184,216,248,280,343,407,471,535,790,1046,1557,2068,3090,4112,6156,8200
 * NB : 1,1,7,11,15,18,19,19,31,31,43,59,67,71,72,216,324,216,396
 */
#define ATL_stGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 120) (nb_) = 1; \
   else if ((n_) < 168) (nb_) = 7; \
   else if ((n_) < 200) (nb_) = 11; \
   else if ((n_) < 232) (nb_) = 15; \
   else if ((n_) < 264) (nb_) = 18; \
   else if ((n_) < 375) (nb_) = 19; \
   else if ((n_) < 503) (nb_) = 31; \
   else if ((n_) < 662) (nb_) = 43; \
   else if ((n_) < 918) (nb_) = 59; \
   else if ((n_) < 1301) (nb_) = 67; \
   else if ((n_) < 1812) (nb_) = 71; \
   else if ((n_) < 2579) (nb_) = 72; \
   else if ((n_) < 3601) (nb_) = 216; \
   else if ((n_) < 5134) (nb_) = 324; \
   else if ((n_) < 7178) (nb_) = 216; \
   else (nb_) = 396; \
}


#endif    /* end ifndef ATL_stGetNB_geqrf */
