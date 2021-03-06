# doc/manpages/Makefile
#
# Part of the NDNx distribution.
#
# Portions Copyright (C) 2013 Regents of the University of California.
# 
# Based on the CCNx C Library by PARC.
# Copyright (C) 2010, 2013 Palo Alto Research Center, Inc.
#
# This work is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License version 2 as published by the
# Free Software Foundation.
# This work is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.
#

# The order here determines the order in the generated index.
PAGE_NAMES =        		\
	ndn_ndnbtoxml		\
	ndn_xmltondnb		\
	ndnacl			\
	ndngroup		\
	ndnc			\
	ndnd			\
	ndndc			\
	ndndcontrol		\
	ndndlogging		\
	ndndsmoketest		\
	ndndstart		\
	ndndstatus		\
	ndndstop		\
	ndnexplore		\
	ndnls			\
	ndnlsrepo		\
	ndngetfile		\
	ndnputfile		\
	ndnputmeta		\
	ndngetmeta		\
	ndnlibtest		\
	ndnnamelist		\
	ndnrm			\
	ndnseqwriter		\
	ndn_repo		\
	ndncat			\
	ndnslurp		\
	ndnchat			\
	ndnpeek			\
	ndnpoke			\
	ndnr			\
	ndnrpolicyedit		\
	ndnsyncslice		\
	ndnsyncwatch		\
	ndnlink			\
	ndnprintlink		\
	ndnrepoimport		\
	ndntestloop 		\
        ndnd-autoconfig \


PROJECT_NUMBER = 0.7.2
SRC_TXT     = $(PAGE_NAMES:=.1.txt)
DERIVED_HTML= $(PAGE_NAMES:=.1.html)
DERIVED_PDF = $(PAGE_NAMES:=.1.pdf)
DERIVED_MAN = $(PAGE_NAMES:=.1)

DERIVED_INDEX = index.html index.txt
DEBRIS = $(PAGE_NAMES:=.1.fo) $(PAGE_NAMES:=.1.xml)
NONSOURCE = $(DERIVED_HTML) $(DERIVED_PDF) $(DERIVED_INDEX) \
	$(DEBRIS)
SRC_HTML = # any checked-in html files should go here
INDEXED_HTML = $(DERIVED_HTML) $(SRC_HTML)

default:
	: To build the documentation: $(MAKE) documentation

documentation dist-docs: html

all: manpages

manpages: $(DERIVED_MAN)

html: $(DERIVED_HTML) $(DERIVED_INDEX)

pdf: $(DERIVED_PDF)

clean:
	@rm -f $(DEBRIS)
	: To clean the documentation: $(MAKE) clean-documentation

clean-documentation:
	rm -f $(NONSOURCE)

install: manpages
	@test -d $(DINST_MAN)/man1 || (echo $(DINST_MAN)/man1 does not exist.  Please mkdir -p $(DINST_MAN)/man1 if this is what you intended. && exit 2)
	for i in $(DERIVED_MAN); do \
           $(INSTALL) -g 0 -o 0 -m 0644 $$i $(DINST_MAN)/man1; \
	done

uninstall:
	for i in $(DERIVED_MAN); do test -z "$$i" || $(RM) $(DINST_MAN)/man1/$$i.gz; done

.gitignore: Makefile
	@for i in $(NONSOURCE); do echo $$i; done > $@

depend: .gitignore _always
	@for i in $(PAGE_NAMES); do echo $$i.1.html: $$i.1.txt; done > depend.mk+
	@for i in $(PAGE_NAMES); do echo $$i.1.pdf: $$i.1.txt; done >> depend.mk+
	@for i in $(PAGE_NAMES); do echo $$i.1: $$i.1.txt;     done >> depend.mk+
	@diff depend.mk depend.mk+ || mv depend.mk+ depend.mk
	@rm -f depend.mk+

index.html: index.txt
	asciidoc index.txt

index.txt: $(INDEXED_HTML) .genindex.sh
	rm -f index.html
	sh .genindex.sh $(INDEXED_HTML) > index.txt

$(DERIVED_HTML): $(@:.html=.txt)
	asciidoc -a revnumber="$(PROJECT_NUMBER)" -d manpage $(@:.html=.txt)

$(DERIVED_PDF): $(@:.pdf=.txt)
	a2x -a revnumber="$(PROJECT_NUMBER)" -f pdf -d manpage $(@:.pdf=.txt)

$(DERIVED_MAN): $(@:=.txt)
	a2x -a revnumber="$(PROJECT_NUMBER)" -f manpage -d manpage $(@:=.txt)

default all clean clean-documentation test check shared documentation dist-docs testinstall install uninstall: _always

_always:

.PHONY: _always depend manpages html

include depend.mk
