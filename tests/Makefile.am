bin_PROGRAMS = test

test_SOURCES = commas.cc  rdms.cc  test.cc  tuples.cc  recordset.cc fields.cc select.cc

INCLUDES = -I$(top_srcdir)

EXTRA_DIST = commas.h  rdms.h  recordset.h  tuples.h

LDADD = \
../$(LIBRARY_NAME)/.libs/libtmplsql.a -lpq -lcppunit -lIceUtil