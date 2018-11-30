#ifndef ATL_dGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,76,127,178,230,281,332,383,435,640,846,1257,1668,2490,3312,4956,6600
 * N : 25,76,127,178,230,281,332,383,435,640,846,1257,1668,2490,3312,4956,6600
 * NB : 1,1,4,10,11,27,35,32,67,56,112,120,224,224,280,448,616
 */
#define ATL_dGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 101) (nb_) = 1; \
   else if ((n_) < 152) (nb_) = 4; \
   else if ((n_) < 204) (nb_) = 10; \
   else if ((n_) < 255) (nb_) = 11; \
   else if ((n_) < 306) (nb_) = 27; \
   else if ((n_) < 357) (nb_) = 35; \
   else if ((n_) < 409) (nb_) = 32; \
   else if ((n_) < 537) (nb_) = 67; \
   else if ((n_) < 743) (nb_) = 56; \
   else if ((n_) < 1051) (nb_) = 112; \
   else if ((n_) < 1462) (nb_) = 120; \
   else if ((n_) < 2901) (nb_) = 224; \
   else if ((n_) < 4134) (nb_) = 280; \
   else if ((n_) < 5778) (nb_) = 448; \
   else (nb_) = 616; \
}


#endif    /* end ifndef ATL_dGetNB_geqrf */
