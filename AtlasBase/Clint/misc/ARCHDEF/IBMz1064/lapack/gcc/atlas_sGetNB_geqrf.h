#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,216,288,360,504,792,864,936,1008,1080,2160
 * N : 25,216,288,360,504,792,864,936,1008,1080,2160
 * NB : 4,12,20,16,24,20,24,28,28,72,72
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 120) (nb_) = 4; \
   else if ((n_) < 252) (nb_) = 12; \
   else if ((n_) < 324) (nb_) = 20; \
   else if ((n_) < 432) (nb_) = 16; \
   else if ((n_) < 648) (nb_) = 24; \
   else if ((n_) < 828) (nb_) = 20; \
   else if ((n_) < 900) (nb_) = 24; \
   else if ((n_) < 1044) (nb_) = 28; \
   else (nb_) = 72; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
