ROUTINE parastress;
   PARAMS :: i0, d0, i1, i2, d1, i3, d2, d3, d4, i4, i5, i6, d5, i7, d6, d7,
             i8, d8, d9, i9, i10, i11, i12, i13, i14, d10, d11, d12, d13, d14,
             iret;
   INT     :: i0, i1, i2, i3, i4, i5, i6, i7, i8, i9, i10, i11, i12, i13, i14;
   INT_PTR :: iret;
   DOUBLE  :: d0, d1, d2, d3, d4, d5, d6, d7, d8, d9, d10, d11, d12, d13, d14;
ROUT_BEGIN
   i9 += i14;
   i9 += i13;
   i9 += i12;
   i9 += i11;
   i9 += i10;
   i9 += i8;
   i9 += i7;
   i9 += i6;
   i9 += i5;
   i9 += i4;
   i9 += i3;
   i9 += i2;
   i9 += i1;
   i9 += i0;
   d9 += d0;
   d9 += d1;
   d9 += d2;
   d9 += d3;
   d9 += d4;
   d9 += d5;
   d9 += d6;
   d9 += d7;
   d9 += d8;
   d9 += d10;
   d9 += d11;
   d9 += d12;
   d9 += d13;
   d9 += d14;
   iret[0] = i9;
   RETURN d9;
ROUT_END
