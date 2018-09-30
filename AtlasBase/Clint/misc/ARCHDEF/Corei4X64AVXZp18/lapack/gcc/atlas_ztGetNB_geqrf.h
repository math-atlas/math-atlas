#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,172,320,467,615,910,1206,1797,2388,2979,3570,4752,7116,9480
 * N : 25,172,320,467,615,910,1206,1797,2388,2979,3570,4752,7116,9480
 * NB : 1,1,27,33,35,67,115,115,131,135,372,372,558,558
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 246) (nb_) = 1; \
   else if ((n_) < 393) (nb_) = 27; \
   else if ((n_) < 541) (nb_) = 33; \
   else if ((n_) < 762) (nb_) = 35; \
   else if ((n_) < 1058) (nb_) = 67; \
   else if ((n_) < 2092) (nb_) = 115; \
   else if ((n_) < 2683) (nb_) = 131; \
   else if ((n_) < 3274) (nb_) = 135; \
   else if ((n_) < 5934) (nb_) = 372; \
   else (nb_) = 558; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
