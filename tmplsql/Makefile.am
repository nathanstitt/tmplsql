
h_sources =  commas.h  fields.h  handle.h  lexical_cast.h  operators.h  quote.h  rdms.h  recordset.h  row_saver.h row_saver_base.h query.h  tmplsql.h functors.h hash_map.h
cc_sources =  commas.cc  handle.cc  rdms.cc fields.cc

INCLUDES = -I$(top_srcdir) -I$(top_builddir)

lib_LTLIBRARIES= libtmplsql.la

libtmplsql_includedir=$(includedir)/$(LIBRARY_NAME)

libtmplsql_include_HEADERS = $(h_sources)

libtmplsql_la_SOURCES= $(h_sources) $(cc_sources)

libtmplsql_la_LDFLAGS= -version-info $(LIBRARY_VERSION) -release $(RELEASE)

