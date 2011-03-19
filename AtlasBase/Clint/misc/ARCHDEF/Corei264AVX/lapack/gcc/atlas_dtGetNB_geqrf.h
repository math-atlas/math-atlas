#ifndef ATL_dtGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,156,312,468,520,572,676,1404,2808,2860,2964,3016,3068,3120,3172,3224,3276,3484,4212,5668
 * N : 25,156,312,468,520,572,676,1404,2808,2860,2964,3016,3068,3120,3172,3224,3276,3484,4212,5668
 * NB : 8,52,52,52,52,56,56,56,40,48,48,48,48,52,48,48,80,56,52,52
 */
#define ATL_dtGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 90) (nb_) = 8; \
   else if ((n_) < 546) (nb_) = 52; \
   else if ((n_) < 2106) (nb_) = 56; \
   else if ((n_) < 2834) (nb_) = 40; \
   else if ((n_) < 3094) (nb_) = 48; \
   else if ((n_) < 3146) (nb_) = 52; \
   else if ((n_) < 3250) (nb_) = 48; \
   else if ((n_) < 3380) (nb_) = 80; \
   else if ((n_) < 3848) (nb_) = 56; \
   else (nb_) = 52; \
}


#endif    /* end ifndef ATL_dtGetNB_geqrf */
