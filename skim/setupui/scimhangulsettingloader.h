/***************************************************************************
 *   Copyright (C) 2003-2005 by liuspider                                          *
 *   sharecash@163.net                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef SCIMHANGULSETTINGLOADER_H
#define SCIMHANGULSETTINGLOADER_H

#include "utils/kautocmodule.h"
/**
@author spider
*/
class ScimHangulSettingLoader : public KAutoCModule
{
Q_OBJECT
public:
    ScimHangulSettingLoader(QWidget *parent, 
  const char */*name*/, const QStringList &args);
    ~ScimHangulSettingLoader();

private:
    class ScimHangulSettingLoaderPrivate;
    ScimHangulSettingLoaderPrivate * d;
};

#endif
