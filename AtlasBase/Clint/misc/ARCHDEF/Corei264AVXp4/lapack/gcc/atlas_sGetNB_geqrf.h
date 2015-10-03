#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,95,165,305,445,586,726,867,1148,2272,4520
 * N : 25,95,165,305,445,586,726,867,1148,2272,4520
 * NB : 12,12,24,24,24,36,48,80,96,144,164
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 130) (nb_) = 12; \
   else if ((n_) < 515) (nb_) = 24; \
   else if ((n_) < 656) (nb_) = 36; \
   else if ((n_) < 796) (nb_) = 48; \
   else if ((n_) < 1007) (nb_) = 80; \
   else if ((n_) < 1710) (nb_) = 96; \
   else if ((n_) < 3396) (nb_) = 144; \
   else (nb_) = 164; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
