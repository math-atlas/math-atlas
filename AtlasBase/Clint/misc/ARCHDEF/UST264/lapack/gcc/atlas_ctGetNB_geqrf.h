#ifndef ATL_ctGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,60,120,160,180,200,220,320,380,400,420,440,460,480,540,640,860,1280,1720,3440
 * N : 25,60,120,160,180,200,220,320,380,400,420,440,460,480,540,640,860,1280,1720,3440
 * NB : 9,12,12,12,12,20,20,20,20,40,60,60,100,100,100,100,120,120,160,160
 */
#define ATL_ctGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 42) (nb_) = 9; \
   else if ((n_) < 190) (nb_) = 12; \
   else if ((n_) < 390) (nb_) = 20; \
   else if ((n_) < 410) (nb_) = 40; \
   else if ((n_) < 450) (nb_) = 60; \
   else if ((n_) < 750) (nb_) = 100; \
   else if ((n_) < 1500) (nb_) = 120; \
   else (nb_) = 160; \
}


#endif    /* end ifndef ATL_ctGetNB_geqrf */
