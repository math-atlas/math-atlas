#ifndef ATL_ctGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,96,167,310,453,524,596,1168,2312,3456,3527,3599,3670,3742,4028,4600
 * N : 25,96,167,310,453,524,596,1168,2312,3456,3527,3599,3670,3742,4028,4600
 * NB : 12,16,24,32,32,36,36,44,56,68,80,80,80,168,168,168
 */
#define ATL_ctGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 60) (nb_) = 12; \
   else if ((n_) < 131) (nb_) = 16; \
   else if ((n_) < 238) (nb_) = 24; \
   else if ((n_) < 488) (nb_) = 32; \
   else if ((n_) < 882) (nb_) = 36; \
   else if ((n_) < 1740) (nb_) = 44; \
   else if ((n_) < 2884) (nb_) = 56; \
   else if ((n_) < 3491) (nb_) = 68; \
   else if ((n_) < 3706) (nb_) = 80; \
   else (nb_) = 168; \
}


#endif    /* end ifndef ATL_ctGetNB_geqrf */
