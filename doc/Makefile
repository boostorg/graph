LATEX		= pdflatex
LATEXOUT	= pdf

.tex.$(LATEXOUT):
	@ if test ! -f $*.ind; then echo "" > $*.ind; fi
	@ $(LATEX) $*
	@ if ( grep 'LaTeX Warning: Label(s) may' $*.log > /dev/null ); \
	then $(LATEX) $* ; fi
	@ if ( grep 'LaTeX Warning: Citation' $*.log > /dev/null ); \
	then bibtex $* ; $(LATEX) $* ; fi
	@ if ( grep 'LaTeX Warning: Label(s) may' $*.log > /dev/null ); \
	then $(LATEX) $* ; fi
	@ if ( grep 'LaTeX Warning: Label(s) may' $*.log > /dev/null ); \
	then $(LATEX) $* ; fi
	@ if ( grep 'Writing index file' $*.log > /dev/null ); \
	then makeindex $* ; $(LATEX) $* ; fi
	@ if ( grep 'LaTeX Warning: Label(s) may' $*.log > /dev/null ); \
	then $(LATEX) $* ; fi
	@ if ( grep 'LaTeX Warning: Label(s) may' $*.log > /dev/null ); \
	then $(LATEX) $* ; fi

.dvi.ps:
	$(DVIPS) -t letter -o $*.ps $*

.eps.pdf:
	epstopdf $*.eps

.dot.eps:
	dot -Tps -o $*.eps $*.dot 

default: isomorphism-impl.$(LATEXOUT) out.pdf in.pdf
	csh ./mungeaux.csh *.aux
	$(LATEX) isomorphism-impl

isomorphism-impl.tex: isomorphism-impl.w
	jweb -np isomorphism-impl.w



clean:
	rm *.aux *.dvi
