#ifndef ATL_cGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,140,256,314,372,430,488,604,720,836,952,1184,1416,1648,1880
 * N : 25,140,256,314,372,430,488,604,720,836,952,1184,1416,1648,1880
 * NB : 1,1,11,11,12,12,15,15,16,16,23,23,24,32,35
 */
#define ATL_cGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 198) (nb_) = 1; \
   else if ((n_) < 343) (nb_) = 11; \
   else if ((n_) < 459) (nb_) = 12; \
   else if ((n_) < 662) (nb_) = 15; \
   else if ((n_) < 894) (nb_) = 16; \
   else if ((n_) < 1300) (nb_) = 23; \
   else if ((n_) < 1532) (nb_) = 24; \
   else if ((n_) < 1764) (nb_) = 32; \
   else (nb_) = 35; \
}


#endif    /* end ifndef ATL_cGetNB_geqrf */
