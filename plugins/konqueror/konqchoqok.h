/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2010-2011 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.


    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, see http://www.gnu.org/licenses/

*/

#ifndef KONQCHOQOK_H
#define KONQCHOQOK_H

#include <kparts/plugin.h>
#include <QDBusInterface>

class KonqPluginChoqok : public KParts::Plugin
{
    Q_OBJECT
public:
    KonqPluginChoqok( QObject* parent, const QVariantList& );
    
    virtual ~KonqPluginChoqok();
    
private Q_SLOTS:
    void slotpostSelectedText();
    void toggleShortening( bool );
    void updateActions();
    
private:
    QDBusInterface *m_interface;

};

#endif // KONQCHOQOK_H
