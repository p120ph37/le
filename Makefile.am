SUBDIRS = doc lib src misc m4
EXTRA_DIST = le.lsm FEATURES HISTORY le.spec
AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS = -I m4

dist-hook:
	@echo $(PACKAGE).spec updating...
	@rm -f $(distdir)/$(PACKAGE).spec
	@-sed "s/%define version .*/%define version $(VERSION)/" \
	    $(srcdir)/$(PACKAGE).spec > $(distdir)/$(PACKAGE).spec && \
	    cp $(distdir)/$(PACKAGE).spec $(srcdir)/$(PACKAGE).spec
	@echo $(PACKAGE).lsm updating...
	@rm -f $(distdir)/$(PACKAGE).lsm
	@-SIZE=`$(AMTAR) chof - $(distdir) | GZIP=$(GZIP_ENV) gzip -c | wc -c`; SIZE=`expr \( $$SIZE + 512 \) / 1024`; echo Size: $$SIZE kB; \
	 DATE=`date +%Y-%m-%d`; echo Date: $$DATE; \
	 sed -e "s/Version:	.*/Version:	$(VERSION)/" \
	     -e "s/[0-9]* kB $(PACKAGE)-.*\.tar\.gz/$$SIZE kB $(PACKAGE)-$(VERSION).tar.gz/" \
	     -e "s/Entered-date:	.*/Entered-date:	$$DATE/" \
	    $(srcdir)/$(PACKAGE).lsm > $(distdir)/$(PACKAGE).lsm && \
	    cp $(distdir)/$(PACKAGE).lsm $(srcdir)/$(PACKAGE).lsm

REL_DIR=/home/lav/www/download/le
release:
	make distcheck
	mv $(PACKAGE)-$(VERSION).tar.gz $(REL_DIR)
	cp -p $(PACKAGE).lsm $(REL_DIR)/$(PACKAGE)-$(VERSION).lsm
	cd $(REL_DIR) && \
	    (addbz $(PACKAGE)-$(VERSION).tar.gz; \
	    gpg -ba $(PACKAGE)-$(VERSION).tar.gz; \
	    gpg -ba $(PACKAGE)-$(VERSION).tar.bz2; \
	    gpg -ba $(PACKAGE)-$(VERSION).tar.xz; \
	    md5sum $(PACKAGE)-$(VERSION).tar.gz $(PACKAGE)-$(VERSION).tar.bz2 $(PACKAGE)-$(VERSION).tar.xz > $(PACKAGE)-$(VERSION).md5sum)
	mutt -s add lsm@execpc.com < $(PACKAGE).lsm
