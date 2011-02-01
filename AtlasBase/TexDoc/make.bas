#
# xdvi -paper usr
#
@whiledef rt atlas_contrib atlas_devel
@(rt).pdf : @(rt).tex
	pdflatex @(rt) ; pdflatex @(rt)
	rm -f @(rt).aux @(rt).dvi @(rt).log \
              @(rt).toc texput.log
@(rt).ps : @(rt).tex
	latex @(rt) ; latex @(rt) ; \
        dvips -o @(rt).ps @(rt) ; \
	rm -f @(rt).aux @(rt).dvi @(rt).log \
              @(rt).toc texput.log
@endwhile
@whiledef rt cblasqref f77blasqref lapackqref
@(rt)_pdf.ps :
	latex @(rt) ; latex @(rt)
	dvips -P pdf -t letter -t landscape -o $@ @(rt)
@(rt).pdf : @(rt)_pdf.ps
	ps2pdf14 @(rt)_pdf.ps @(rt).pdf
	rm -f @(rt).aux @(rt).dvi @(rt).log \
              @(rt).toc texput.log @(rt)_pdf.ps
@(rt).ps : @(rt).tex
	latex @(rt) ; latex @(rt) ; \
        dvips -tlandscape -o @(rt).ps @(rt) ; \
	rm -f @(rt).aux @(rt).dvi @(rt).log \
              @(rt).toc texput.log
@endwhile
