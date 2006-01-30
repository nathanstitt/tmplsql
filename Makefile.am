
if MAKE_TESTS
SUBDIRS = tmplsql scripts tests doc
else
SUBDIRS = tmplsql scripts doc
endif

m4datadir = $(datadir)/aclocal
m4data_DATA = tmplsql.m4

EXTRA_DIST = tmplsql.m4


doc-dummy:

doc: doc-dummy
	$(MAKE) -C doc doc
