
F77_TS_dir   = $(TSTdir)/F77/
C_TS_dir     = $(TSTdir)/C/
CTS_dir      = $(TOPdir)/interfaces/blas/C/testing
F77TS_dir    = $(TOPdir)/interfaces/blas/F77/testing

bindir       = $(TOPdir)/bin/

CSHELL       = /bin/csh

all      : init run san clean cleanexe

init     : F77blasinit     Cblasinit     bininit

install  :
	cp $(SRCdir)/F77/?blat?.* $(F77_TS_dir)/.
	cp $(SRCdir)/C/c_?blat?.* $(C_TS_dir)/.
#	cp $(SRCdir)/F77/sblat2.short $(F77TS_dir)/sblat2.dat
#	cp $(SRCdir)/F77/dblat2.short $(F77TS_dir)/dblat2.dat
#	cp $(SRCdir)/F77/cblat2.short $(F77TS_dir)/cblat2.dat
#	cp $(SRCdir)/F77/zblat2.short $(F77TS_dir)/zblat2.dat
#	cp $(SRCdir)/F77/sblat3.short $(F77TS_dir)/sblat3.dat
#	cp $(SRCdir)/F77/dblat3.short $(F77TS_dir)/dblat3.dat
#	cp $(SRCdir)/F77/cblat3.short $(F77TS_dir)/cblat3.dat
#	cp $(SRCdir)/F77/zblat3.short $(F77TS_dir)/zblat3.dat
#	cp $(SRCdir)/C/c_sblat2.short $(CTS_dir)/c_sblat2.dat
#	cp $(SRCdir)/C/c_dblat2.short $(CTS_dir)/c_dblat2.dat
#	cp $(SRCdir)/C/c_cblat2.short $(CTS_dir)/c_cblat2.dat
#	cp $(SRCdir)/C/c_zblat2.short $(CTS_dir)/c_zblat2.dat
#	cp $(SRCdir)/C/c_sblat3.short $(CTS_dir)/c_sblat3.dat
#	cp $(SRCdir)/C/c_dblat3.short $(CTS_dir)/c_dblat3.dat
#	cp $(SRCdir)/C/c_cblat3.short $(CTS_dir)/c_cblat3.dat
#	cp $(SRCdir)/C/c_zblat3.short $(CTS_dir)/c_zblat3.dat

F77test  : F77blasinit     F77blassan    F77blasrun    F77blasclean \
           F77blascleanexe

Ctest    : Cblasinit       Cblassan      Cblasrun      Cblasclean   \
           Cblascleanexe

bintest  : bininit                       binrun        binclean     \
           bincleanexe

san      : F77blassan      Cblassan

run      : binrun F77blasrun      Cblasrun     

clean    : F77blasclean    Cblasclean    binclean

cleanexe : F77blascleanexe Cblascleanexe bincleanexe

cleanres :
	( cd $(TSTdir)/res/; rm -f * )

F77blasinit         :
	- rm -f $(F77_TS_dir)/Make.inc
	ln -s $(TOPdir)/Make.inc $(F77_TS_dir)/Make.inc
	- rm -f $(F77_TS_dir)/Makefile
	cp $(SRCdir)/makes/Make.f77blastst $(F77_TS_dir)/Makefile
	( cd $(F77_TS_dir); $(MAKE) all TSRCdir=$(SRCdir) )
	- rm -f $(F77_TS_dir)/f77blasrun
	ln -s $(SRCdir)/makes/f77blasrun $(F77_TS_dir)/f77blasrun
	- rm -f $(F77_TS_dir)/f77blassan
	ln -s $(SRCdir)/makes/f77blassan $(F77_TS_dir)/f77blassan
F77blassan          :
	$(MAKE) install
	( cd $(F77_TS_dir); $(CSHELL) f77blassan $(TOPdir) $(ARCH) )
	- rm -f $(F77_TS_dir)/f77blassan
F77blasrun          :
	( cd $(F77_TS_dir); $(CSHELL) f77blasrun $(TOPdir) $(ARCH) )
	- rm -f $(F77_TS_dir)/f77blasrun
F77blasclean        :
	- ( cd $(F77_TS_dir); $(MAKE) clean );
	- ( cd $(TOPdir)/interfaces/blas/F77/testing/; $(MAKE) clean );
F77blascleanexe     :
	- ( cd $(F77_TS_dir); $(MAKE) cleanexe );
#	- ( cd $(F77_TS_dir); rm -f *.dat );
	- rm -rf $(TSTdir)/F77/*
	- ( cd $(TOPdir)/interfaces/blas/F77/testing/; $(MAKE) clean );
	- ( cd $(TOPdir)/interfaces/blas/F77/testing/; rm -f x* );

Cblasinit           :
	- rm -f $(C_TS_dir)/Make.inc
	ln -s $(TOPdir)/Make.inc $(C_TS_dir)/Make.inc
	- rm -f $(C_TS_dir)/Makefile
	ln -s $(SRCdir)/makes/Make.cblastst $(C_TS_dir)/Makefile
	( cd $(C_TS_dir); $(MAKE) all TSRCdir=$(SRCdir) )
	- rm -f $(C_TS_dir)/cblasrun
	ln -s $(SRCdir)/makes/cblasrun $(C_TS_dir)/cblasrun
	- rm -f $(C_TS_dir)/cblassan
	ln -s $(SRCdir)/makes/cblassan $(C_TS_dir)/cblassan
Cblassan            :
	$(MAKE) install
	( cd $(C_TS_dir); $(CSHELL) cblassan $(TOPdir) $(ARCH) )
	- rm -f $(C_TS_dir)/cblassan
Cblasrun            :
	( cd $(C_TS_dir); $(CSHELL) cblasrun $(TOPdir) $(ARCH) )
	- rm -f $(C_TS_dir)/cblasrun
Cblasclean          :
	- ( cd $(C_TS_dir); $(MAKE) clean );
	- ( cd $(TOPdir)/interfaces/blas/C/testing/; $(MAKE) clean );
Cblascleanexe       :
	- ( cd $(C_TS_dir); $(MAKE) cleanexe );
#	- ( cd $(C_TS_dir); rm -f *.dat );
	- rm -rf $(TSTdir)/C/*
	- ( cd $(TOPdir)/interfaces/blas/C/testing/; $(MAKE) clean );
	- ( cd $(TOPdir)/interfaces/blas/C/testing/; rm -f x* );

bininit             : 
	( cd $(bindir); $(MAKE) xsl1blastst   )
	( cd $(bindir); $(MAKE) xdl1blastst   )
	( cd $(bindir); $(MAKE) xcl1blastst   )
	( cd $(bindir); $(MAKE) xzl1blastst   )
	( cd $(bindir); $(MAKE) xsl2blastst   )
	( cd $(bindir); $(MAKE) xdl2blastst   )
	( cd $(bindir); $(MAKE) xcl2blastst   )
	( cd $(bindir); $(MAKE) xzl2blastst   )
	( cd $(bindir); $(MAKE) xsmmtst       )
	( cd $(bindir); $(MAKE) xdmmtst       )
	( cd $(bindir); $(MAKE) xcmmtst       )
	( cd $(bindir); $(MAKE) xzmmtst       )
	( cd $(bindir); $(MAKE) xsl3blastst   )
	( cd $(bindir); $(MAKE) xdl3blastst   )
	( cd $(bindir); $(MAKE) xcl3blastst   )
	( cd $(bindir); $(MAKE) xzl3blastst   )
	( cd $(bindir); $(MAKE) xsllttst     )
	( cd $(bindir); $(MAKE) xdllttst      )
	( cd $(bindir); $(MAKE) xcllttst      )
	( cd $(bindir); $(MAKE) xzllttst      )
	( cd $(bindir); $(MAKE) xslutst       )
	( cd $(bindir); $(MAKE) xdlutst       )
	( cd $(bindir); $(MAKE) xclutst       )
	( cd $(bindir); $(MAKE) xzlutst       )
	( cd $(bindir); $(MAKE) xsqrtst       )
	( cd $(bindir); $(MAKE) xdqrtst       )
	( cd $(bindir); $(MAKE) xcqrtst       )
	( cd $(bindir); $(MAKE) xzqrtst       )
	( cd $(bindir); $(MAKE) xsinvtst      )
	( cd $(bindir); $(MAKE) xdinvtst      )
	( cd $(bindir); $(MAKE) xcinvtst      )
	( cd $(bindir); $(MAKE) xzinvtst      )
	- grep "NCPU" $(TOPdir)/Make.inc > res/ncpu.out
	if [ -s res/ncpu.out ]; then \
	   ( cd $(bindir); $(MAKE) xsmmtst_pt       ); \
	   ( cd $(bindir); $(MAKE) xdmmtst_pt       ); \
	   ( cd $(bindir); $(MAKE) xcmmtst_pt       ); \
	   ( cd $(bindir); $(MAKE) xzmmtst_pt       ); \
	   ( cd $(bindir); $(MAKE) xsl3blastst_pt   ); \
	   ( cd $(bindir); $(MAKE) xdl3blastst_pt   ); \
	   ( cd $(bindir); $(MAKE) xcl3blastst_pt   ); \
	   ( cd $(bindir); $(MAKE) xzl3blastst_pt   ); \
	   ( cd $(bindir); $(MAKE) xsllttst_pt     ); \
	   ( cd $(bindir); $(MAKE) xdllttst_pt     ); \
	   ( cd $(bindir); $(MAKE) xcllttst_pt     ); \
	   ( cd $(bindir); $(MAKE) xzllttst_pt     ); \
	   ( cd $(bindir); $(MAKE) xslutst_pt      ); \
	   ( cd $(bindir); $(MAKE) xdlutst_pt      ); \
	   ( cd $(bindir); $(MAKE) xclutst_pt      ); \
	   ( cd $(bindir); $(MAKE) xzlutst_pt      ); \
	   ( cd $(bindir); $(MAKE) xsqrtst_pt      ); \
	   ( cd $(bindir); $(MAKE) xdqrtst_pt      ); \
	   ( cd $(bindir); $(MAKE) xcqrtst_pt      ); \
	   ( cd $(bindir); $(MAKE) xzqrtst_pt      ); \
	   ( cd $(bindir); $(MAKE) xsinvtst_pt      ); \
	   ( cd $(bindir); $(MAKE) xdinvtst_pt      ); \
	   ( cd $(bindir); $(MAKE) xcinvtst_pt      ); \
	   ( cd $(bindir); $(MAKE) xzinvtst_pt      ); \
	fi
	- rm -f $(bindir)/binrun
	ln -s $(SRCdir)/makes/binrun $(bindir)/binrun
	if [ -f /usr/bin/uname ]; then \
	   if [ "`/usr/bin/uname -m`" = "alpha" ]; then \
	      rm -f $(bindir)/Make.inc ; \
	      ln -s $(TOPdir)/Make.inc $(bindir)/Make.inc ; \
	   fi \
	fi
#	if [ -f /usr/bin/uname ]; then \
#	   if [ "`/usr/bin/uname -m`" = "alpha" ]; then \
#	      rm -f $(bindir)/Make.inc ; \
#	      sed -e 's/FLINKFLAGS = /FLINKFLAGS = -nofor_main /' \
#                 $(TOPdir)/Make.$(ARCH) > $(bindir)/Make.inc ; \
#	   fi \
#	fi
binrun              :
	( cd $(bindir); $(CSHELL) binrun $(ARCH) $(bindir) $(TSTdir) )
	- rm -f $(bindir)/binrun
binclean            :
	- ( cd $(bindir); $(MAKE) clean );
bincleanexe         :
	- ( cd $(bindir); rm -f x* );
