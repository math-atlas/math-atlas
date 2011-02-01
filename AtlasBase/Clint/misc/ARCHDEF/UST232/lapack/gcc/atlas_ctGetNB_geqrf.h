#ifndef ATL_ctGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,96,192,288,352,384,416,448,512,640,864,1728,3456
 * N : 25,96,192,288,352,384,416,448,512,640,864,1728,3456
 * NB : 9,12,16,16,16,32,64,128,128,128,128,128,128
 */
#define ATL_ctGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 60) (nb_) = 9; \
   else if ((n_) < 144) (nb_) = 12; \
   else if ((n_) < 368) (nb_) = 16; \
   else if ((n_) < 400) (nb_) = 32; \
   else if ((n_) < 432) (nb_) = 64; \
   else (nb_) = 128; \
}


#endif    /* end ifndef ATL_ctGetNB_geqrf */
