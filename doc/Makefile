#LATEX		= latex
#LATEXOUT	= tex

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

.w.tex: /afs/nd.edu/user11/jsiek/.jweb
	jweb -np $*.w

BOOST		= ../../..
CXX		= c++
INCLUDES	= -I$(BOOST)
CXXFLAGS	= -Wall -g -Wno-long-double -ftemplate-depth-30 $(INCLUDES)

.cpp.o:
	$(CXX) $(CXXFLAGS) $< -c


JWEB	= isomorphism-impl.w
DOT	= out.dot in.dot
EPS_GEN	= $(DOT:.dot=.eps)
WEBTEX	= $(JWEB:.w=.tex)
PDFPICT	= $(EPS_GEN:.eps=.pdf)

default: isomorphism-impl.$(LATEXOUT)
	$(LATEX) isomorphism-impl
	csh ./mungeaux.csh *.aux
	$(LATEX) isomorphism-impl

isomorphism-impl.dvi: $(EPS_GEN) $(WEBTEX)
isomorphism-impl.ps: isomorphism-impl.dvi
isomorphism-impl.pdf: $(PDFPICT) $(WEBTEX)
isomorphism.hpp: isomorphism-impl.w isomorphism-impl.tex

isomorphism-impl-v2.dvi: $(EPS_GEN) $(WEBTEX)
isomorphism-impl-v2.ps: isomorphism-impl-v2.dvi
isomorphism-impl-v2.pdf: $(PDFPICT) $(WEBTEX)
isomorphism-v2.hpp: isomorphism-impl-v2.w isomorphism-impl-v2.tex

out.dot: isomorphism-impl.w
in.dot: isomorphism-impl.w

isomorphism.o: isomorphism-v2.hpp

isomorphism: isomorphism.o
	$(CXX) $(CXXFLAGS) $< -o $@ $(LIBS)

clean:
	rm *.aux *.dvi $(EPS_GEN) $(PDFPICT)
