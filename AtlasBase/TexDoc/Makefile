#
# xdvi -paper usr
#
atlas_install.pdf : atlas_install.tex
	pdflatex atlas_install ; pdflatex atlas_install
	- bibtex atlas_install
	pdflatex atlas_install ; pdflatex atlas_install
	rm -f atlas_install.aux atlas_install.dvi atlas_install.log \
              atlas_install.toc atlas_install.bbl atlas_install.blg \
              texput.log
atlas_install_html:
	latex atlas_install ; latex atlas_install
	- bibtex atlas_install
	latex atlas_install ; latex atlas_install
	latex2html atlas_install
	rm -f atlas_install.aux atlas_install.dvi atlas_install.log \
              atlas_install.toc atlas_install.bbl atlas_install.blg \
              texput.log
extract.pdf : extract.tex
	pdflatex extract ; pdflatex extract
	pdflatex extract ; pdflatex extract
	rm -f extract.aux extract.dvi extract.log \
              extract.toc texput.log
atlas_devel.pdf : atlas_devel.tex
	pdflatex atlas_devel ; pdflatex atlas_devel
	rm -f atlas_devel.aux atlas_devel.dvi atlas_devel.log \
              atlas_devel.toc texput.log
atlas_devel_html : atlas_devel.tex
	latex atlas_devel ; latex atlas_devel ; \
        latex2html atlas_devel
	rm -f atlas_devel.aux atlas_devel.dvi atlas_devel.log \
              atlas_devel.toc texput.log
atlas_devel.ps : atlas_devel.tex
	latex atlas_devel ; latex atlas_devel ; \
        dvips -o atlas_devel.ps atlas_devel ; \
	rm -f atlas_devel.aux atlas_devel.dvi atlas_devel.log \
              atlas_devel.toc texput.log
atlas_contrib.pdf : atlas_contrib.tex
	pdflatex atlas_contrib ; pdflatex atlas_contrib
	rm -f atlas_contrib.aux atlas_contrib.dvi atlas_contrib.log \
              atlas_contrib.toc texput.log
atlas_contrib.ps : atlas_contrib.tex
	latex atlas_contrib ; latex atlas_contrib ; \
        dvips -o atlas_contrib.ps atlas_contrib ; \
	rm -f atlas_contrib.aux atlas_contrib.dvi atlas_contrib.log \
              atlas_contrib.toc texput.log
atlas_contrib_html : atlas_contrib.tex
	latex atlas_contrib ; latex atlas_contrib ; \
	latex2html atlas_contrib
	rm -f atlas_contrib.aux atlas_contrib.dvi atlas_contrib.log \
              atlas_contrib.toc texput.log
lapackqref_pdf.ps :
	latex lapackqref ; latex lapackqref
	dvips -P pdf -t letter -t landscape -o $@ lapackqref
lapackqref.pdf : lapackqref_pdf.ps
	ps2pdf14 lapackqref_pdf.ps lapackqref.pdf
	rm -f lapackqref.aux lapackqref.dvi lapackqref.log \
              lapackqref.toc texput.log lapackqref_pdf.ps
lapackqref.ps : lapackqref.tex
	latex lapackqref ; latex lapackqref ; \
        dvips -tlandscape -o lapackqref.ps lapackqref ; \
	rm -f lapackqref.aux lapackqref.dvi lapackqref.log \
              lapackqref.toc texput.log
f77blasqref_pdf.ps :
	latex f77blasqref ; latex f77blasqref
	dvips -P pdf -t letter -t landscape -o $@ f77blasqref
f77blasqref.pdf : f77blasqref_pdf.ps
	ps2pdf14 f77blasqref_pdf.ps f77blasqref.pdf
	rm -f f77blasqref.aux f77blasqref.dvi f77blasqref.log \
              f77blasqref.toc texput.log f77blasqref_pdf.ps
f77blasqref.ps : f77blasqref.tex
	latex f77blasqref ; latex f77blasqref ; \
        dvips -tlandscape -o f77blasqref.ps f77blasqref ; \
	rm -f f77blasqref.aux f77blasqref.dvi f77blasqref.log \
              f77blasqref.toc texput.log
cblasqref_pdf.ps :
	latex cblasqref ; latex cblasqref
	dvips -P pdf -t letter -t landscape -o $@ cblasqref
cblasqref.pdf : cblasqref_pdf.ps
	ps2pdf14 cblasqref_pdf.ps cblasqref.pdf
	rm -f cblasqref.aux cblasqref.dvi cblasqref.log \
              cblasqref.toc texput.log cblasqref_pdf.ps
cblasqref.ps : cblasqref.tex
	latex cblasqref ; latex cblasqref ; \
        dvips -tlandscape -o cblasqref.ps cblasqref ; \
	rm -f cblasqref.aux cblasqref.dvi cblasqref.log \
              cblasqref.toc texput.log
