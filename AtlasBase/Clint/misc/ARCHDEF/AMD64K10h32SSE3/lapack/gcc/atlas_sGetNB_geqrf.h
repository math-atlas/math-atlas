#ifndef ATL_sGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='LOWER'
 * M : 25,50,75,100,125,150,175,200,250,300,350,400,450,500,600,700,800,900,1000,1200,1400,1600,1800,2000
 * N : 25,50,75,100,125,150,175,200,250,300,350,400,450,500,600,700,800,900,1000,1200,1400,1600,1800,2000
 * NB : 4,8,16,12,16,12,16,16,16,12,24,40,40,28,40,24,40,60,40,40,40,128,128,128
 */
#define ATL_sGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 37) (nb_) = 4; \
   else if ((n_) < 62) (nb_) = 8; \
   else if ((n_) < 87) (nb_) = 16; \
   else if ((n_) < 112) (nb_) = 12; \
   else if ((n_) < 137) (nb_) = 16; \
   else if ((n_) < 162) (nb_) = 12; \
   else if ((n_) < 275) (nb_) = 16; \
   else if ((n_) < 325) (nb_) = 12; \
   else if ((n_) < 375) (nb_) = 24; \
   else if ((n_) < 475) (nb_) = 40; \
   else if ((n_) < 550) (nb_) = 28; \
   else if ((n_) < 650) (nb_) = 40; \
   else if ((n_) < 750) (nb_) = 24; \
   else if ((n_) < 850) (nb_) = 40; \
   else if ((n_) < 950) (nb_) = 60; \
   else if ((n_) < 1500) (nb_) = 40; \
   else (nb_) = 128; \
}


#endif    /* end ifndef ATL_sGetNB_geqrf */
