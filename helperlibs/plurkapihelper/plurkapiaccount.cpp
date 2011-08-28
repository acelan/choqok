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

#include "plurkapiaccount.h"
#include "plurkapimicroblog.h"
#include <passwordmanager.h>
#include <QtOAuth/QtOAuth>
#include <kio/accessmanager.h>
#include <KDebug>

class PlurkApiAccount::Private
{
public:
    Private()
        :api('/'), usingOauth(true), qoauth(0)
    {}
    QString userId;
    int count;
    QString host;
    QString api;
    KUrl apiUrl;
    KUrl homepageUrl;
    QStringList friendsList;
    QStringList timelineNames;
    QByteArray oauthToken;
    QByteArray oauthTokenSecret;
    QByteArray oauthConsumerKey;
    QByteArray oauthConsumerSecret;
    bool usingOauth;
    QOAuth::Interface *qoauth;
};

PlurkApiAccount::PlurkApiAccount(PlurkApiMicroBlog* parent, const QString &alias)
    : Account(parent, alias), d(new Private)
{
    kDebug();
    d->usingOauth = configGroup()->readEntry("UsingOAuth", false);
    d->userId = configGroup()->readEntry("UserId", QString());
    d->count = configGroup()->readEntry("CountOfPosts", 20);
    d->host = configGroup()->readEntry("Host", QString());
    d->friendsList = configGroup()->readEntry("Friends", QStringList());
    d->timelineNames = configGroup()->readEntry("Timelines", QStringList());
    d->oauthToken = configGroup()->readEntry("OAuthToken", QByteArray());
    d->oauthConsumerKey = configGroup()->readEntry("OAuthConsumerKey", QByteArray());
    d->oauthConsumerSecret = Choqok::PasswordManager::self()->readPassword(
                                            QString("%1_consumerSecret").arg(alias) ).toUtf8();
    d->oauthTokenSecret = Choqok::PasswordManager::self()->readPassword(
                                            QString("%1_tokenSecret").arg(alias) ).toUtf8();
    setApi( configGroup()->readEntry("Api", QString('/') ) );

    kDebug()<<"UsingOAuth: "<<d->usingOauth;
    if(d->usingOauth){
        initQOAuthInterface();
    }

    if( this->d->userId.isEmpty() ) {
        // FIXME this is an asynchronized method
        // so the fallowing listFriendsUsername call may not work
        parent->getProfile( this );
    }

    if( d->timelineNames.isEmpty() ){
        QStringList list = parent->timelineNames();
        list.removeOne("Public");
        list.removeOne("Favorite");
        list.removeOne("ReTweets");
        d->timelineNames = list;
    }

    if( d->friendsList.isEmpty() ){
        parent->listFriendsUsername(this);
        //Result will set on PlurkApiMicroBlog!
    }

}

PlurkApiAccount::~PlurkApiAccount()
{
    delete d;
}

void PlurkApiAccount::writeConfig()
{
    configGroup()->writeEntry("UsingOAuth", d->usingOauth);
    configGroup()->writeEntry("UserId", d->userId);
    configGroup()->writeEntry("CountOfPosts", d->count);
    configGroup()->writeEntry("Host", d->host);
    configGroup()->writeEntry("Api", d->api);
    configGroup()->writeEntry("Friends", d->friendsList);
    configGroup()->writeEntry("Timelines", d->timelineNames);
    configGroup()->writeEntry("OAuthToken", d->oauthToken );
    configGroup()->writeEntry("OAuthConsumerKey", d->oauthConsumerKey );
    Choqok::PasswordManager::self()->writePassword( QString("%1_consumerSecret").arg(alias()),
                                                    QString::fromUtf8(d->oauthConsumerSecret) );
    Choqok::PasswordManager::self()->writePassword( QString("%1_tokenSecret").arg(alias()),
                                                    QString::fromUtf8( d->oauthTokenSecret) );
    Choqok::Account::writeConfig();
}

QString PlurkApiAccount::userId() const
{
    return d->userId;
}

void PlurkApiAccount::setUserId( const QString &id )
{
    d->userId = id;
}

int PlurkApiAccount::countOfPosts() const
{
    return d->count;
}

void PlurkApiAccount::setCountOfPosts(int count)
{
    d->count = count;
}

KUrl PlurkApiAccount::apiUrl() const
{
    return d->apiUrl;
}

QString PlurkApiAccount::host() const
{
    return d->host;
}

void PlurkApiAccount::setApiUrl(const KUrl& apiUrl)
{
    d->apiUrl = apiUrl;
}

QString PlurkApiAccount::api() const
{
    return d->api;
}

void PlurkApiAccount::setApi(const QString& api)
{
    d->api = api;
    generateApiUrl();
}

void PlurkApiAccount::setHost(const QString& host)
{
    d->host = host;
    generateApiUrl();
}

KUrl PlurkApiAccount::homepageUrl() const
{
    return d->homepageUrl;
}

void PlurkApiAccount::generateApiUrl()
{
    if(!host().startsWith(QLatin1String("http")))//NOTE: This is for compatibility by prev versions. remove it after 1.0 release
        setHost(host().prepend("http://"));
    KUrl url(host());

    setHomepageUrl(url);

    url.addPath(api());
    setApiUrl(url);
}

void PlurkApiAccount::setHomepageUrl(const KUrl& homepageUrl)
{ 
    d->homepageUrl = homepageUrl;
}

QStringList PlurkApiAccount::friendsList() const
{
    return d->friendsList;
}

void PlurkApiAccount::setFriendsList(const QStringList& list)
{
    d->friendsList = list;
    writeConfig();
}

QStringList PlurkApiAccount::timelineNames() const
{
    return d->timelineNames;
}

void PlurkApiAccount::setTimelineNames(const QStringList& list)
{
    d->timelineNames.clear();
    foreach(const QString &name, list){
        if(microblog()->timelineNames().contains(name))
            d->timelineNames<<name;
    }
}

QByteArray PlurkApiAccount::oauthToken() const
{
    return d->oauthToken;
}

void PlurkApiAccount::setOauthToken(const QByteArray& token)
{
    d->oauthToken = token;
}

QByteArray PlurkApiAccount::oauthTokenSecret() const
{
    return d->oauthTokenSecret;
}

void PlurkApiAccount::setOauthTokenSecret(const QByteArray& tokenSecret)
{
    d->oauthTokenSecret = tokenSecret;
}

QByteArray PlurkApiAccount::oauthConsumerKey() const
{
    return d->oauthConsumerKey;
}

void PlurkApiAccount::setOauthConsumerKey(const QByteArray& consumerKey)
{
    d->oauthConsumerKey = consumerKey;
}

QByteArray PlurkApiAccount::oauthConsumerSecret() const
{
    return d->oauthConsumerSecret;
}

void PlurkApiAccount::setOauthConsumerSecret(const QByteArray& consumerSecret)
{
    d->oauthConsumerSecret = consumerSecret;
}

bool PlurkApiAccount::usingOAuth() const
{
    return d->usingOauth;
}

void PlurkApiAccount::setUsingOAuth(bool use)
{
    if(use)
        initQOAuthInterface();
    else{
        delete d->qoauth;
        d->qoauth = 0L;
    }
    d->usingOauth = use;
}

QOAuth::Interface* PlurkApiAccount::oauthInterface()
{
    return d->qoauth;
}

void PlurkApiAccount::initQOAuthInterface()
{
    kDebug();
    if(!d->qoauth)
        d->qoauth = new QOAuth::Interface(new KIO::AccessManager(this), this);//TODO KDE 4.5 Change to use new class.
    d->qoauth->setConsumerKey(d->oauthConsumerKey);
    d->qoauth->setConsumerSecret(d->oauthConsumerSecret);
    d->qoauth->setRequestTimeout(20000);
    d->qoauth->setIgnoreSslErrors(true);
}

#include "plurkapiaccount.moc"
