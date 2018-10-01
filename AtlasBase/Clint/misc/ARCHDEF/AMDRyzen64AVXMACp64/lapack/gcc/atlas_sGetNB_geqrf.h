#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,60,96,131,167,202,238,274,310,453,596,882,1168,1454,1740,2026,2312,3456,4600
 * N : 25,60,96,131,167,202,238,274,310,453,596,882,1168,1454,1740,2026,2312,3456,4600
 * NB : 1,3,4,12,19,19,20,20,27,24,51,63,67,83,115,139,163,211,259
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 42) (nb_) = 1; \
   else if ((n_) < 78) (nb_) = 3; \
   else if ((n_) < 113) (nb_) = 4; \
   else if ((n_) < 149) (nb_) = 12; \
   else if ((n_) < 220) (nb_) = 19; \
   else if ((n_) < 292) (nb_) = 20; \
   else if ((n_) < 381) (nb_) = 27; \
   else if ((n_) < 524) (nb_) = 24; \
   else if ((n_) < 739) (nb_) = 51; \
   else if ((n_) < 1025) (nb_) = 63; \
   else if ((n_) < 1311) (nb_) = 67; \
   else if ((n_) < 1597) (nb_) = 83; \
   else if ((n_) < 1883) (nb_) = 115; \
   else if ((n_) < 2169) (nb_) = 139; \
   else if ((n_) < 2884) (nb_) = 163; \
   else if ((n_) < 4028) (nb_) = 211; \
   else (nb_) = 259; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
