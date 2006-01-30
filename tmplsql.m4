dnl ---------------------------------------------------------------------------
dnl Macros for tmplsql detection. Typically used in configure.in as:
dnl
dnl 	AC_ARG_ENABLE(...)
dnl 	AC_ARG_WITH(...)
dnl	...
dnl	AM_OPTIONS_TMPLSQLCONFIG
dnl	...
dnl	...
dnl	AM_PATH_TMPLSQLCONFIG(2.3.2, tmplsql=1)
dnl     if test "$tmplsql" != 1; then
dnl        AC_MSG_ERROR([
dnl     	   tmplsql must be installed on your system
dnl     	   but tmplsql-config script couldn't be found.
dnl     
dnl     	   Please check that tmplsql-config is in path, the directory
dnl     	   where tmplsql libraries are installed (returned by
dnl     	   'tmplsql-config --libs' command) is in LD_LIBRARY_PATH or
dnl     	   equivalent variable and tmplsql version is 2.3.2 or above.
dnl        ])
dnl     fi
dnl     CPPFLAGS="$CPPFLAGS $TMPLSQL_CPPFLAGS"
dnl     CXXFLAGS="$CXXFLAGS $TMPLSQL_CXXFLAGS_ONLY"
dnl     CFLAGS="$CFLAGS $TMPLSQL_CFLAGS_ONLY"
dnl     
dnl     LDFLAGS="$LDFLAGS $TMPLSQL_LIBS"
dnl ---------------------------------------------------------------------------

dnl ---------------------------------------------------------------------------
dnl AM_OPTIONS_TMPLSQLCONFIG
dnl
dnl adds support for --tmplsql-prefix, --tmplsql-exec-prefix and --tmplsql-config 
dnl command line options
dnl ---------------------------------------------------------------------------

AC_DEFUN(AM_OPTIONS_TMPLSQLCONFIG,
[
   AC_ARG_WITH(tmplsql-config,[  --with-tmplsql-config=CONFIG   tmplsql-config script to use (optional)],
               tmplsql_config_name="$withval", tmplsql_config_name="")
])

dnl ---------------------------------------------------------------------------
dnl AM_PATH_TMPLSQLCONFIG(VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl
dnl Test for tmplsql, and define TMPLSQL_C*FLAGS, TMPLSQL_LIBS and TMPLSQL_LIBS_STATIC
dnl (the latter is for static linking against tmplsql). Set TMPLSQL_CONFIG_NAME
dnl environment variable to override the default name of the tmplsql-config script
dnl to use. Set TMPLSQL_CONFIG_PATH to specify the full path to tmplsql-config - in this
dnl case the macro won't even waste time on tests for its existence.
dnl ---------------------------------------------------------------------------

dnl
dnl Get the cflags and libraries from the tmplsql-config script
dnl
AC_DEFUN(AM_PATH_TMPLSQLCONFIG,
[

    TMPLSQL_CONFIG_NAME=tmplsql-config
    if test "x$tmplsql_config_name" != x ; then
	TMPLSQL_CONFIG_NAME="$tmplsql_config_name"
    fi
    if test -x "$TMPLSQL_CONFIG_NAME" ; then
	TMPLSQL_CONFIG_PATH="$TMPLSQL_CONFIG_NAME"
	AC_MSG_RESULT($TMPLSQL_CONFIG_PATH)
    else
	AC_PATH_PROG(TMPLSQL_CONFIG_PATH, $TMPLSQL_CONFIG_NAME, no, $PATH)
    fi


    min_tmplsql_version=ifelse([$1], ,0.7.0,$1)
    AC_MSG_CHECKING(for tmplsql version >= $min_tmplsql_version)

    TMPLSQL_CONFIG_WITH_ARGS="$TMPLSQL_CONFIG_PATH $tmplsql_config_args"

    TMPLSQL_VERSION=`$TMPLSQL_CONFIG_WITH_ARGS --version`

    tmplsql_config_major_version=`echo $TMPLSQL_VERSION | \
	sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    tmplsql_config_minor_version=`echo $TMPLSQL_VERSION | \
	sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    tmplsql_config_micro_version=`echo $TMPLSQL_VERSION | \
	sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`

    tmplsql_requested_major_version=`echo $min_tmplsql_version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    tmplsql_requested_minor_version=`echo $min_tmplsql_version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    tmplsql_requested_micro_version=`echo $min_tmplsql_version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`

    tmplsql_ver_ok=""
    if test $tmplsql_config_major_version -gt $tmplsql_requested_major_version; then
      tmplsql_ver_ok=yes
    else
	if test $tmplsql_config_major_version -eq $tmplsql_requested_major_version; then
	    if test $tmplsql_config_minor_version -gt $tmplsql_requested_minor_version; then
		tmplsql_ver_ok=yes
	    else
		if test $tmplsql_config_minor_version -eq $tmplsql_requested_minor_version; then
		    if test $tmplsql_config_micro_version -ge $tmplsql_requested_micro_version; then
			tmplsql_ver_ok=yes
		    fi
		fi
	    fi
	fi
    fi
    if test "x$tmplsql_ver_ok" = x ; then
	AC_MSG_RESULT(no (version $TMPLSQL_VERSION is not new enough))
	TMPLSQL_CFLAGS=""
	TMPLSQL_CPPFLAGS=""
	TMPLSQL_CXXFLAGS=""
	TMPLSQL_LIBS=""
	TMPLSQL_LIBS_STATIC=""
	AC_MSG_RESULT(yes (version $TMPLSQL_VERSION))
    else
	ifelse([$2], , :, [$2])
        AC_MSG_RESULT(yes (version $TMPLSQL_VERSION))
	TMPLSQL_LIBS=`$TMPLSQL_CONFIG_WITH_ARGS --libs`
	TMPLSQL_LIBS_STATIC=`$TMPLSQL_CONFIG_WITH_ARGS --static --libs`

	TMPLSQL_CFLAGS=`$TMPLSQL_CONFIG_WITH_ARGS --cflags`
	TMPLSQL_CPPFLAGS=$TMPLSQL_CFLAGS
	TMPLSQL_CXXFLAGS=$TMPLSQL_CFLAGS
    fi
    AC_SUBST(TMPLSQL_CPPFLAGS)
    AC_SUBST(TMPLSQL_CFLAGS)
    AC_SUBST(TMPLSQL_CXXFLAGS)
    AC_SUBST(TMPLSQL_LIBS)
    AC_SUBST(TMPLSQL_LIBS_STATIC)
    AC_SUBST(TMPLSQL_VERSION)
])
