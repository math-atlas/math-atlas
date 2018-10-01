#ifndef ATL_ztGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,155,285,317,350,382,415,447,480,512,545,675,805,837,870,902,935,967,1000,1033,1066,1326,1587,1652,1717,1782,1847,1977,2108,2629,2889,3019,3150,3215,3280,3345,3410,3540,3671,3801,3931,4061,4192,4452,4713,4778,4843,4908,4973,5103,5234,5364,5494,5624,5755,6015,6276,7318,8360
 * N : 25,155,285,317,350,382,415,447,480,512,545,675,805,837,870,902,935,967,1000,1033,1066,1326,1587,1652,1717,1782,1847,1977,2108,2629,2889,3019,3150,3215,3280,3345,3410,3540,3671,3801,3931,4061,4192,4452,4713,4778,4843,4908,4973,5103,5234,5364,5494,5624,5755,6015,6276,7318,8360
 * NB : 1,1,7,15,19,20,23,23,24,26,27,27,28,30,32,33,40,40,41,41,43,43,44,44,46,46,48,48,51,48,44,44,63,63,65,69,71,71,75,76,77,78,83,83,87,87,89,89,90,90,91,91,93,93,94,94,95,96,99
 */
#define ATL_ztGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 220) (nb_) = 1; \
   else if ((n_) < 301) (nb_) = 7; \
   else if ((n_) < 333) (nb_) = 15; \
   else if ((n_) < 366) (nb_) = 19; \
   else if ((n_) < 398) (nb_) = 20; \
   else if ((n_) < 463) (nb_) = 23; \
   else if ((n_) < 496) (nb_) = 24; \
   else if ((n_) < 528) (nb_) = 26; \
   else if ((n_) < 740) (nb_) = 27; \
   else if ((n_) < 821) (nb_) = 28; \
   else if ((n_) < 853) (nb_) = 30; \
   else if ((n_) < 886) (nb_) = 32; \
   else if ((n_) < 918) (nb_) = 33; \
   else if ((n_) < 983) (nb_) = 40; \
   else if ((n_) < 1049) (nb_) = 41; \
   else if ((n_) < 1456) (nb_) = 43; \
   else if ((n_) < 1684) (nb_) = 44; \
   else if ((n_) < 1814) (nb_) = 46; \
   else if ((n_) < 2042) (nb_) = 48; \
   else if ((n_) < 2368) (nb_) = 51; \
   else if ((n_) < 2759) (nb_) = 48; \
   else if ((n_) < 3084) (nb_) = 44; \
   else if ((n_) < 3247) (nb_) = 63; \
   else if ((n_) < 3312) (nb_) = 65; \
   else if ((n_) < 3377) (nb_) = 69; \
   else if ((n_) < 3605) (nb_) = 71; \
   else if ((n_) < 3736) (nb_) = 75; \
   else if ((n_) < 3866) (nb_) = 76; \
   else if ((n_) < 3996) (nb_) = 77; \
   else if ((n_) < 4126) (nb_) = 78; \
   else if ((n_) < 4582) (nb_) = 83; \
   else if ((n_) < 4810) (nb_) = 87; \
   else if ((n_) < 4940) (nb_) = 89; \
   else if ((n_) < 5168) (nb_) = 90; \
   else if ((n_) < 5429) (nb_) = 91; \
   else if ((n_) < 5689) (nb_) = 93; \
   else if ((n_) < 6145) (nb_) = 94; \
   else if ((n_) < 6797) (nb_) = 95; \
   else if ((n_) < 7839) (nb_) = 96; \
   else (nb_) = 99; \
}


#endif    /* end ifndef ATL_ztGetNB_geqrf */
