main()
{
   double fsimple(double,double);
   float f0=3.0, f1 = 2.2, fret;
   fret = fsimple(f0, f1);
   printf("%f+%f+2.5 = %f (%f)\n", f0, f1, fret, f0+f1+2.5);
}
