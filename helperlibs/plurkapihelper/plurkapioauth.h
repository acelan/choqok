
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

#ifndef PLURKAPIOAUTH_H
#define PLURKAPIOAUTH_H

#include <QWidget>
#include <QtOAuth/QtOAuth>
#include <QtOAuth/interface.h>
#include <QtOAuth/qoauth_namespace.h>

#include "choqok_export.h"

class CHOQOK_EXPORT PlurkApiOAuth : public QWidget
{
    Q_OBJECT
public:
    ~PlurkApiOAuth();
    static PlurkApiOAuth* self();

    bool authorizeUser();
    bool getPinCode();

    QByteArray oauthToken() const;
    void setOAuthToken( const QByteArray &token );

    QByteArray oauthTokenSecret() const;
    void setOAuthTokenSecret( const QByteArray &tokenSecret );

    QByteArray oauthConsumerKey() const;
    void setOAuthConsumerKey( const QByteArray &consumerKey );

    QByteArray oauthConsumerSecret() const;
    void setOAuthConsumerSecret( const QByteArray &consumerSecret );

    QOAuth::Interface *oauthInterface();

protected:
    QString username;

    QByteArray _oauthToken;
    QByteArray _oauthTokenSecret;
    QByteArray _oauthConsumerKey;
    QByteArray _oauthConsumerSecret;
    QOAuth::Interface *qoauth;

    void initQOAuthInterface();

private:
    PlurkApiOAuth();
    static PlurkApiOAuth* mSelf;
};

#endif
