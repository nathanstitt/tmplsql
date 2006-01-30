dnl@synposis RSSH_CHECK_CORBA_ORB 
dnl
dnl set CORBA support for omniORB v3-pr2 or highter
dnl    ( http://www.uk.research.att.com/omniORB/omniORB.html)
dnl
dnl@author (C) Ruslan Shevchenko <Ruslan@Shevchenko.Kiev.UA>, 1999, 2000
dnl@id $Id: omniorb.m4 2 2002-08-24 19:02:47Z nas $
dnl
AC_DEFUN([AM_CHECK_OMNIORB],[
AC_REQUIRE([AC_PROG_CXX])dnl
AC_REQUIRE([AC_PROG_CPP])dnl
AC_REQUIRE([AC_PROG_CXXCPP])dnl

AC_LANG_SAVE
AC_LANG_CPLUSPLUS

AC_ARG_WITH(omniORB-prefix, [  --with-omniORB-prefix=PREFIX prefix to omniORB installation (default: /usr/local)] ,\
            OMNI_ROOT="$withval" , OMNI_ROOT=/usr/local )

AC_ARG_WITH(omniORB-include, [  --with-omniORB-include=PATH path to omniORB header files (default: omniORB-prefix/include)] ,\
            OMNI_INCLUDE="$withval" , OMNI_INCLUDE=$OMNI_ROOT/include )

AC_ARG_WITH(omniORB-libs, [  --with-omniORB-libs=PATH path to omniORB shared object files (default: omniORB-prefix/lib)] ,\
            OMNI_LIBS="$withval" , OMNI_LIBS=$OMNI_ROOT/lib )

AC_CHECKING(for omniORB4)

CPPFLAGS="-D__OMNIORB4__ -D__x86__ -D_REENTRANT -D__linux__ -D__OSVERSION__=2 ${CPPFLAGS}"
AC_CHECK_HEADER(omniORB4/CORBA.h, ,[ AC_MSG_ERROR( omniORB4/CORBA.h header file not found ) ] )



if test ! -r "$OMNI_LIBS/libomniORB4.so"
then
	AC_MSG_ERROR("Cannot find shared object OMNI_LIBS/libomniORB4.so")
else
	OMNI_LIBS="-L$OMNI_LIBS -lomniORB4"
fi

OMNI_LIBS="$OMNI_LIBS -lomnithread -lpthread"

case $target_cpu in
	sparc*)
	    OMNI_CFLAGS="$OMNI_CFLAGS -D__sparc__"
	;;
	"i686"|"i586"|"i486"|"i386")
		OMNI_CFLAGS="$OMNI_CFLAGS -D__x86__"
	;;
esac


case $target_os in
	solaris*)
		OMNI_LIBS="$OMNI_LIBS -D__sunos__ -D__OSVERSION__=5 "
	;;
	freebsd*)
		OMNI_LIBS="$OMNI_LIBS -D__freebsd__ "
	;;
	linux-gnu*)
		OMNI_LIBS="$OMNI_LIBS -D_REENTRANT -D__linux__  -D__OSVERSION__=2 "
	;;
esac



AC_CHECK_LIB(pthread,pthread_create,LIBS="-lpthread")

LIBS="$OMNI_CFLAGS $OMNI_LIBS"

AC_CACHE_CHECK(for omnithreads,
    check_omnithreads,
    AC_TRY_LINK(
#include <omnithread.h>
,omni_mutex my_mutex,
                 check_omnithreads=yes,check_omnithreads=no)
)


if  test ! $check_omnithreads = yes
then
	AC_MSG_ERROR("unable to link with omnithreads - was omnthread lib created with the same compiler you are useing?")
	omni=no
fi

AC_CHECK_LIB(socket,socket, LIBS="-lsocket $LIBS",,)
AC_CHECK_LIB(nsl,gethostbyname, LIBS="-lnsl $LIBS",,)




AC_CACHE_CHECK(whether we can link with omniORB4,
    check_omniORBlib,
    AC_TRY_LINK(
#include <omniORB4/CORBA.h>
,CORBA::ORB_var orb,
    check_omniORBlib=yes,check_omniORBlib=no
    )
  )


if  test ! $cv_check_omniORBlib = yes
then
    AC_MSG_ERROR("omniORB library linking failed - was omniORB lib created with the same compiler you are useing?")
    omni=no
fi


CORBA_LIBS=$OMNI_LIBS
CORBA_CFLAGS=$OMNI_CFLAGS

AC_SUBST(CORBA_LIBS)
AC_SUBST(CORBA_CFLAGS)

AC_LANG_RESTORE

])dnl
