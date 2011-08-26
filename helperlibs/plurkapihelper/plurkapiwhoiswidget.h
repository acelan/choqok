/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2008-2011 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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

#ifndef PLURKAPIWHOISWIDGET_H
#define PLURKAPIWHOISWIDGET_H

#include <QtCore/QUrl>
#include <QtGui/QFrame>
#include "choqok_export.h"
#include <choqoktypes.h>

namespace Choqok {
class Account;
}

class PlurkApiAccount;
class KJob;
class CHOQOK_HELPER_EXPORT PlurkApiWhoisWidget : public QFrame
{
    Q_OBJECT
public:
    PlurkApiWhoisWidget( PlurkApiAccount* theAccount, const QString &userName,
                           const Choqok::Post &post, QWidget *parent=0 );
    ~PlurkApiWhoisWidget();
    void show(QPoint pos);

protected Q_SLOTS:
    void checkAnchor( const QUrl url );
    void userInfoReceived( KJob *job );
    void slotCancel();
    void avatarFetchError( const QString &remoteUrl, const QString &errMsg );
    void avatarFetched( const QString &remoteUrl, const QPixmap &pixmap );

    void slotFriendshipCreated(Choqok::Account*, const QString&);
    void slotFriendshipDestroyed(Choqok::Account*, const QString&);
//     void slotUserBlocked(Choqok::Account*, const QString&);

protected:
    void updateHtml();
    void setActionImages();

//     static const QString baseText;
private:
    void setupUi();
    void showForm();
    void loadUserInfo( PlurkApiAccount* thAccount, const QString& username );
    class Private;
    Private * const d;
};

#endif // PLURKAPIWHOISWIDGET_H
