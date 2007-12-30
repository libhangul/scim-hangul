## Makefile.am -- Process this file with automake to produce Makefile.in
## Copyright (C) 2002 James Su
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2, or (at your option)
## any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

MAINTAINERCLEANFILES	= Makefile.in

hangul_DEFINES		= -DSCIM_ICONDIR=\"@SCIM_ICONDIR@\" \
			  -DSCIM_HANGUL_LOCALEDIR=\"$(localedir)\" \
			  -DSCIM_HANGUL_DATADIR=\"@SCIM_HANGUL_DATADIR@\" \
			  -DSCIM_HANGUL_ICON_FILE=\"@SCIM_ICONDIR@/scim-hangul.png\"

noinst_HEADERS		= scim_hangul_imengine.h

moduledir		= @SCIM_MODULEDIR@/IMEngine
module_LTLIBRARIES	= hangul.la

hangul_la_SOURCES 	= scim_hangul_imengine.cpp

hangul_la_CFLAGS	= @SCIM_CFLAGS@ @LIBHANGUL_CFLAGS@ $(hangul_DEFINES)
hangul_la_CXXFLAGS	= @SCIM_CFLAGS@ @LIBHANGUL_CFLAGS@ $(hangul_DEFINES)

hangul_la_LDFLAGS	= -avoid-version \
		     	  -rpath $(moduledir) \
			  -module \
			  @LIBTOOL_EXPORT_OPTIONS@ \
			  @INTLLIBS@ \
			  @SCIM_LIBS@ \
			  @LIBHANGUL_LIBS@

if SCIM_BUILD_SETUP
SETUP_MODULE = hangul-imengine-setup.la
endif

setupmoduledir           = @SCIM_MODULEDIR@/SetupUI
setupmodule_LTLIBRARIES  = $(SETUP_MODULE)

hangul_imengine_setup_la_SOURCES = scim_hangul_imengine_setup.cpp

hangul_imengine_setup_la_CFLAGS  = @SCIM_GTKUTILS_CFLAGS@ \
				   @SCIM_CFLAGS@ \
				   $(hangul_DEFINES)

hangul_imengine_setup_la_CXXFLAGS= @SCIM_GTKUTILS_CFLAGS@ \
				   @SCIM_CFLAGS@ \
				   $(hangul_DEFINES)

hangul_imengine_setup_la_LDFLAGS = -avoid-version \
				   -rpath $(setupmoduledir) \
				   -module \
				   @LIBTOOL_EXPORT_OPTIONS@ \
				   @SCIM_GTKUTILS_LIBS@ \
				   @INTLLIBS@ \
				   @SCIM_LIBS@
