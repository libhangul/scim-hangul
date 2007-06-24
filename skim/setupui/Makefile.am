INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/src $(all_includes) @SCIM_KDEUTILS_CFLAGS@ 
METASOURCES = AUTO
noinst_HEADERS = scimhangulsettingloader.h
kde_module_LTLIBRARIES = kcm_skimplugin_scim_hangul.la

kcm_skimplugin_scim_hangul_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
kcm_skimplugin_scim_hangul_la_SOURCES = scim_hangul.kcfgc scim_hangul_ui.ui scimhangulsettingloader.cpp
kcm_skimplugin_scim_hangul_la_LIBADD = $(LIB_QT) $(LIB_KDEUI) $(LIB_KDECORE) @SCIM_KDEUTILS_LIBS@

kde_kcfg_DATA = scim_hangul.kcfg

kcm_DATA = skimplugin_scim_hangul_config.desktop

kcmdir = $(kde_servicesdir)/skimconfiguredialog
