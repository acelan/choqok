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

#include <KToolInvocation>
#include <KDebug>
#include <KMessageBox>
#include <KInputDialog>
#include <kio/accessmanager.h>
#include <choqoktools.h>

#include "plurkapioauth.h"

const char* plurkOAuthRequestTokenURL = "http://www.plurk.com/OAuth/request_token";
const char* plurkOAuthAuthorizeURL = "http://www.plurk.com/OAuth/authorize";
const char* plurkOAuthAccessToken = "http://www.plurk.com/OAuth/access_token";
const char* plurkConsumerKey = "4TH0YShIOi3g";
const char* plurkConsumerSecret = "aAdoTJzJQAGcJauJ3dHLYV7LkSEo7tUS";
 
PlurkApiOAuth* PlurkApiOAuth::mSelf = 0L;

PlurkApiOAuth::~PlurkApiOAuth()
{
    if(!qoauth)
        delete qoauth;
}

PlurkApiOAuth* PlurkApiOAuth::self()
{
    if ( !mSelf )
        mSelf = new PlurkApiOAuth;
    return mSelf;
}

PlurkApiOAuth::PlurkApiOAuth() : qoauth(0)
{
    initQOAuthInterface();
}

QByteArray PlurkApiOAuth::oauthToken() const
{
    return _oauthToken;
}

void PlurkApiOAuth::setOAuthToken(const QByteArray& token)
{
    _oauthToken = token;
}

QByteArray PlurkApiOAuth::oauthTokenSecret() const
{
    return _oauthTokenSecret;
}

void PlurkApiOAuth::setOAuthTokenSecret(const QByteArray& tokenSecret)
{
    _oauthTokenSecret = tokenSecret;
}

QByteArray PlurkApiOAuth::oauthConsumerKey() const
{
    return _oauthConsumerKey;
}

void PlurkApiOAuth::setOAuthConsumerKey(const QByteArray& consumerKey)
{
    _oauthConsumerKey = consumerKey;
}

QByteArray PlurkApiOAuth::oauthConsumerSecret() const
{
    return _oauthConsumerSecret;
}

void PlurkApiOAuth::setOAuthConsumerSecret(const QByteArray& consumerSecret)
{
    _oauthConsumerSecret = consumerSecret;
}

QOAuth::Interface* PlurkApiOAuth::oauthInterface()
{
    return qoauth;
}

bool PlurkApiOAuth::authorizeUser()
{
    kDebug();
    // set the consumer key and secret
    qoauth->setConsumerKey( plurkConsumerKey );
    qoauth->setConsumerSecret( plurkConsumerSecret );
    // set a timeout for requests (in msecs)
    qoauth->setRequestTimeout( 20000 );
    qoauth->setIgnoreSslErrors(true);

    QOAuth::ParamMap otherArgs;

    // send a request for an unauthorized token
    QOAuth::ParamMap reply =
        qoauth->requestToken( plurkOAuthRequestTokenURL, QOAuth::GET, QOAuth::HMAC_SHA1 );

    // if no error occurred, read the received token and token secret
    if ( qoauth->error() == QOAuth::NoError ) {
        _oauthToken = reply.value( QOAuth::tokenParameterName() );
        _oauthTokenSecret = reply.value( QOAuth::tokenSecretParameterName() );
        kDebug()<<"token: "<<_oauthToken;
	QUrl url(plurkOAuthAuthorizeURL);
        url.addQueryItem("oauth_token", _oauthToken);
        url.addQueryItem( "oauth_callback", "oob" );
	KToolInvocation::invokeBrowser(url.toString());
        return getPinCode();
    } else {
        kDebug()<<"ERROR: " <<qoauth->error()<<' '<<Choqok::qoauthErrorText(qoauth->error());
        KMessageBox::detailedError(this, "Authorization Error",
                                   Choqok::qoauthErrorText(qoauth->error()));
    }

    return false;
}

bool PlurkApiOAuth::getPinCode()
{
    QString verifier = KInputDialog::getText( "Security code",
                                              "Security code recieved from Plurk",
                                                    "Enter security code:");
    if(verifier.isEmpty())
        return false;
    QOAuth::ParamMap otherArgs;
    otherArgs.insert( "oauth_verifier", verifier.toUtf8() );

    // send a request to exchange Request Token for an Access Token
    QOAuth::ParamMap reply =
    qoauth->accessToken( QString(plurkOAuthAccessToken),
                         QOAuth::GET, _oauthToken, _oauthTokenSecret, QOAuth::HMAC_SHA1, otherArgs );
    // if no error occurred, read the Access Token (and other arguments, if applicable)
    if ( qoauth->error() == QOAuth::NoError ) {
        username = reply.value( "screen_name" );
        _oauthToken = reply.value( QOAuth::tokenParameterName() );
        _oauthTokenSecret = reply.value( QOAuth::tokenSecretParameterName() );
        KMessageBox::information(this, "Choqok is authorized successfully.",
                                 "Authorized");
        return true;
    } else {
        kDebug()<<"ERROR: "<<qoauth->error()<<' '<<Choqok::qoauthErrorText(qoauth->error());
        KMessageBox::detailedError(this, "Authorization Error",
                                Choqok::qoauthErrorText(qoauth->error()));
    }

    return false;
}

void PlurkApiOAuth::initQOAuthInterface()
{
    kDebug();
    if(!qoauth)
        qoauth = new QOAuth::Interface(new KIO::AccessManager(this), this);//TODO KDE 4.5 Change to use new class.
    qoauth->setConsumerKey( plurkConsumerKey);
    qoauth->setConsumerSecret( plurkConsumerSecret);
    qoauth->setRequestTimeout(20000);
    qoauth->setIgnoreSslErrors(true);
}

