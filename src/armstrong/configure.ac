AC_INIT([Armstrong], [0.4])
AM_INIT_AUTOMAKE
LT_INIT

AC_ARG_WITH([jack],
    AS_HELP_STRING([--without-jack], [Ignore presence of jack and disable it]))
AC_ARG_WITH([portmidi],
    AS_HELP_STRING([--without-portmidi], [Ignore presence of portmidi and disable it]))

# check for c++ and stl
AC_PROG_CXX
AC_CHECK_PROG(has_cxx, "$CXX", "yes", "no")
if test "$has_cxx" == "no"
then
	AC_MSG_ERROR([c++ compiler not found])
fi

AC_LANG([C++])
AC_CHECK_LIB(stdc++, main,, AC_MSG_ERROR([libstdc++ not installed]))

# check for boost
AC_CHECK_HEADER([boost/lockfree/queue.hpp], [], [AC_MSG_ERROR([libboost-dev 1.53 or newer not installed])])

AC_CHECK_HEADER([boost/shared_ptr.hpp], [], [AC_MSG_ERROR([libboost-dev not installed])])

AC_CHECK_HEADER([boost/graph/adjacency_list.hpp], [], [AC_MSG_ERROR([libboost-graph-dev not installed])])

# player dependencies
case $host in
	*-*-android*)
		# portmidi and jack are not mandatory on android
		with_jack=no
		with_portmidi=no
		;;
	*)
		AS_IF([test "x$with_jack" != "xno"],
			[AC_CHECK_HEADERS([jack/jack.h], with_jack=yes, [AC_MSG_ERROR(["libjack-dev or libjack-jackd2-dev is not installed"])])],
			with_jack=no
		)
		AS_IF([test "x$with_portmidi" != "xno"],
			[AC_CHECK_HEADERS([portmidi.h], with_portmidi=yes, [AC_MSG_ERROR(["libportmidi-dev is not installed"])])],
			with_portmidi=no
		)
	;;
esac

AS_IF([test "x$with_jack" != "xno"],
	AC_DEFINE(HAVE_JACK, 1), AC_DEFINE(HAVE_JACK, 0) )
AS_IF([test "x$with_portmidi" != "xno"],
	AC_DEFINE(HAVE_PORTMIDI, 1), AC_DEFINE(HAVE_PORTMIDI, 0) )

AM_CONDITIONAL([JACK], [test "x$with_jack" != "xno"])
AM_CONDITIONAL([PORTMIDI], [test "x$with_portmidi" != "xno"])

# storage dependencies
AC_CHECK_HEADER([sqlite3.h], [], [AC_MSG_ERROR(["libsqlite3-dev not installed"])])


# global dependencies
AC_CHECK_HEADER([sndfile.h], [], [AC_MSG_ERROR([libsndfile1-dev not installed])])

AC_CHECK_HEADER([zlib.h], [], [AC_MSG_ERROR([zlib1g-dev not installed])])


AC_CONFIG_FILES([Makefile] [src/player/Makefile] [src/mixing/Makefile] [src/storage/Makefile] [src/plugins/core/Makefile] [src/plugins/midi/Makefile] [src/plugins/ld_fungus/Makefile] [src/plugins/lunar/Makefile] [src/armstrong/Makefile])

AC_OUTPUT

