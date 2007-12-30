# Process this file with autoconf to produce a configure script.
AC_INIT(scim-hangul, 0.3.2, choe.hwanjing@gmail.com) #do not forget to update skim/configure.in.in
AC_CONFIG_SRCDIR([src/scim_hangul_imengine.h])

# Save this value here, since automake will set cflags and cxxflags later
cflags_set=${CFLAGS+set}
cppflags_set=${CPPFLAGS+set}

SCIM_HANGUL_MAJOR_VERSION=0
SCIM_HANGUL_MINOR_VERSION=3
SCIM_HANGUL_MICRO_VERSION=2

SCIM_HANGUL_VERSION=$SCIM_HANGUL_MAJOR_VERSION.$SCIM_HANGUL_MINOR_VERSION.$SCIM_HANGUL_MICRO_VERSION

AC_SUBST(SCIM_HANGUL_MAJOR_VERSION)
AC_SUBST(SCIM_HANGUL_MINOR_VERSION)
AC_SUBST(SCIM_HANGUL_MICRO_VERSION)
AC_SUBST(SCIM_HANGUL_VERSION)

# Init automake stuff
AM_INIT_AUTOMAKE
AM_CONFIG_HEADER([config.h])

# Init gettext
ALL_LINGUAS="de fr ja ko nl pa zh_CN"
AM_GNU_GETTEXT
AM_MKINSTALLDIRS

GETTEXT_PACKAGE=scim-hangul
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [The gettext domain])

# Init libtool
AC_DISABLE_STATIC
AC_LIBTOOL_DLOPEN
AC_DISABLE_STATIC
AC_PROG_LIBTOOL
AC_LIB_LTDL
AC_SUBST(LIBTOOL_DEPS)

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_LANG(C++)

AC_C_CONST
AC_TYPE_SIZE_T

# Checks for libraries.
PKG_CHECK_MODULES(LIBHANGUL,[libhangul >= 0.0.4])

SCIM_VERSION=0.99.8

PKG_CHECK_MODULES(SCIM,[scim >= $SCIM_VERSION])

# Check if we should build setup module
PKG_CHECK_MODULES(SCIM_GTKUTILS,[scim-gtkutils >= $SCIM_VERSION],
		  		[SCIM_HAS_GTKUTILS=yes],
				[SCIM_HAS_GTKUTILS=no])


AC_SUBST(SCIM_VERSION)

AM_CONDITIONAL(SCIM_BUILD_SETUP, [test "$SCIM_HAS_GTKUTILS" = "yes"])

if test "$SCIM_HAS_GTKUTILS" = "yes"; then
  SCIM_BUILD_SETUP=1
else
  SCIM_BUILD_SETUP=0
fi

AC_SUBST(SCIM_BUILD_SETUP)

# Checks for header files.
AC_HEADER_STDC

# Checks for library functions.
AC_SUBST(ac_aux_dir)

# libtool option to control which symbols are exported
# right now, symbols starting with _ are not exported
LIBTOOL_EXPORT_OPTIONS='-export-symbols-regex "^[[^_]].*"'
AC_SUBST(LIBTOOL_EXPORT_OPTIONS)

# Extra args.
AC_ARG_ENABLE(debug,
	      [  --enable-debug          Turn on debugging],
	      enable_debug=yes,
	      enable_debug=no)
if test "$enable_debug" = "yes"; then
    AC_DEFINE(ENABLE_DEBUG, 1 ,[Define this to enable the debug facility in libscim])
    CFLAGS="$CFLAGS -g"
    CXXFLAGS="$CXXFLAGS -g"
fi

AC_ARG_ENABLE(skim-support,
              [  --disable-skim-support  Enable skim Setup UI],
	      skim_support=no,
	      skim_support=yes)

AM_CONDITIONAL(SCIM_BUILD_SKIM_SETUP, [test "$skim_support" = "yes"])

SCIM_ICONDIR=`$PKG_CONFIG --variable=icondir scim`
SCIM_MODULEDIR=`$PKG_CONFIG --variable=moduledir scim`
SCIM_DATADIR=`$PKG_CONFIG --variable=scimdatadir scim`

if test "x$SCIM_ICONDIR" = "x"; then
  SCIM_ICONDIR=${datadir}/scim/icons
fi

if test "x$SCIM_MODULEDIR" = "x"; then
  SCIM_MODULEDIR=${libdir}/scim-1.0
fi

if test "x$SCIM_DATADIR" = "x"; then
  SCIM_DATADIR=${datadir}/scim
fi

AC_SUBST(SCIM_ICONDIR)
AC_SUBST(SCIM_MODULEDIR)
AC_SUBST(SCIM_DATADIR)

SCIM_HANGUL_DATADIR=${SCIM_DATADIR}/hangul
AC_SUBST(SCIM_HANGUL_DATADIR)

AC_CONFIG_FILES([Makefile
		 intl/Makefile
		 po/Makefile.in
		 m4/Makefile
		 src/Makefile
		 data/Makefile
		 scim-hangul.spec])

if test "$skim_support" = "yes"; then
    REQUIRED_SKIM_VERSION=1.2.1
    dnl skim probably has been installed under prefix=$KDEDIR,
    dnl so add it to search path of pkgconfig
    kde_libsuffix=`kde-config --libsuffix`
    test -z "$KDEDIR" && KDEDIR=`kde-config --prefix`
    AC_SUBST(KDEDIR)
    export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$KDEDIR/lib${kde_libsuffix}/pkgconfig
    PKG_CHECK_MODULES(SKIM, [skim >= $REQUIRED_SKIM_VERSION],
			    [HAS_SKIM=yes],
			    [HAS_SKIM=no])
else
    HAS_SKIM=no
fi

if test "x$HAS_SKIM" = "xyes"; then

PKG_CHECK_MODULES(SCIM_KDEUTILS,[scim_kdeutils >= $REQUIRED_SKIM_VERSION],
                                [HAS_KDEUTILS=yes],
                                [HAS_KDEUTILS=no])

  if test "x$HAS_KDEUTILS" = "xyes"; then
      SCIM_HANGUL_ENABLE_SKIM_SUPPORT=1
      AC_CONFIG_SUBDIRS(skim)
  else
      SCIM_HANGUL_ENABLE_SKIM_SUPPORT=0
  fi
fi

AM_CONDITIONAL(SCIM_HANGUL_ENABLE_SKIM_SUPPORT, [test "$HAS_KDEUTILS" = "yes"])
AC_SUBST(SCIM_HANGUL_ENABLE_SKIM_SUPPORT)

AC_OUTPUT
