/***************************************************************************
 *   Copyright (C) 2003-2005 by liuspider                                          *
 *   sharecash@163.net                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "scimhangulsettingloader.h"

#include "scim_hangul.h"
#include "scim_hangul_ui.h"

#include <kgenericfactory.h>
#include <klocale.h>

typedef KGenericFactory<ScimHangulSettingLoader> ScimHangulSettingLoaderFactory;

K_EXPORT_COMPONENT_FACTORY( kcm_skimplugin_scim_hangul, 
    ScimHangulSettingLoaderFactory( "kcm_skimplugin_scim_hangul" ) )

class ScimHangulSettingLoader::ScimHangulSettingLoaderPrivate {
public:
    ScimHangulSetting * ui;
};

ScimHangulSettingLoader::ScimHangulSettingLoader(QWidget *parent, 
  const char */*name*/, const QStringList &args)
 : KAutoCModule( ScimHangulSettingLoaderFactory::instance(), 
     parent, args, ScimHangulConfig::self() ),
   d(new ScimHangulSettingLoaderPrivate)
{
    KGlobal::locale()->insertCatalogue("skim-scim-hangul");
    d->ui = new ScimHangulSetting(this);
    setMainWidget(d->ui);
}

ScimHangulSettingLoader::~ScimHangulSettingLoader()
{
    KGlobal::locale()->removeCatalogue("skim-scim-hangul");
}
#include "scimhangulsettingloader.moc"
