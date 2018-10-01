#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,115,206,251,297,342,388,570,615,661,706,752,1116,1298,1480
 * N : 25,115,206,251,297,342,388,570,615,661,706,752,1116,1298,1480
 * NB : 1,1,11,11,12,12,15,12,20,24,24,27,24,48,67
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 160) (nb_) = 1; \
   else if ((n_) < 274) (nb_) = 11; \
   else if ((n_) < 365) (nb_) = 12; \
   else if ((n_) < 479) (nb_) = 15; \
   else if ((n_) < 592) (nb_) = 12; \
   else if ((n_) < 638) (nb_) = 20; \
   else if ((n_) < 729) (nb_) = 24; \
   else if ((n_) < 934) (nb_) = 27; \
   else if ((n_) < 1207) (nb_) = 24; \
   else if ((n_) < 1389) (nb_) = 48; \
   else (nb_) = 67; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
