all: presentation.pdf

%.pdf: %.tex
	xelatex $<
	bibtex $(basename $<)
	xelatex $<
	xelatex $<

clean:
	-@rm -f *.aux *.log *.toc *.out *.nav *.snm *.vrb *.bbl *.blg *.pdf

.PHONY: all clean
