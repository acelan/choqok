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

#include "twittereditaccount.h"
#include "twittermicroblog.h"
#include "twitteraccount.h"
#include <KDebug>
#include <kio/jobclasses.h>
#include <kio/netaccess.h>
#include <kio/job.h>
#include <KMessageBox>
#include <QDomDocument>
#include <KToolInvocation>
#include <QProgressBar>
#include <accountmanager.h>
#include <choqoktools.h>
#include <QtOAuth/interface.h>
#include <QtOAuth/qoauth_namespace.h>
#include <kio/accessmanager.h>
#include <QCheckBox>
#include <KInputDialog>

const char * twitterConsumerKey = "VyXMf0O7CvciiUQjliYtYg";
const char * twitterConsumerSecret = "uD2HvsOBjzt1Vs6SnouFtuxDeHmvOOVwmn3fBVyCw0";

TwitterEditAccountWidget::TwitterEditAccountWidget(TwitterMicroBlog *microblog,
                                                    TwitterAccount* account, QWidget* parent)
    : ChoqokEditAccountWidget(account, parent), mAccount(account)
{
    setupUi(this);
    kcfg_basicAuth->hide();
    connect(kcfg_authorize, SIGNAL(clicked(bool)), SLOT(authorizeUser()));
    if(mAccount) {
        kcfg_alias->setText( mAccount->alias() );
        if(mAccount->oauthToken().isEmpty() || mAccount->oauthTokenSecret().isEmpty()) {
            setAuthenticated(false);
        } else {
            setAuthenticated(true);
            token = mAccount->oauthToken();
            tokenSecret = mAccount->oauthTokenSecret();
            username = mAccount->username();
        }
    } else {
        setAuthenticated(false);
        QString newAccountAlias = microblog->serviceName();
        QString servName = newAccountAlias;
        int counter = 1;
        while(Choqok::AccountManager::self()->findAccount(newAccountAlias)){
            newAccountAlias = QString("%1%2").arg(servName).arg(counter);
        counter++;
    }
        setAccount( mAccount = new TwitterAccount(microblog, newAccountAlias) );
        kcfg_alias->setText( newAccountAlias );
    }
    loadTimelinesTableState();
    kcfg_alias->setFocus(Qt::OtherFocusReason);
}

TwitterEditAccountWidget::~TwitterEditAccountWidget()
{

}

bool TwitterEditAccountWidget::validateData()
{
    if(kcfg_alias->text().isEmpty() || !isAuthenticated )
        return false;
    else
        return true;
}

Choqok::Account* TwitterEditAccountWidget::apply()
{
    kDebug();
    mAccount->setAlias(kcfg_alias->text());
    mAccount->setUsername( username );
    mAccount->setOauthToken( token );
    mAccount->setOauthTokenSecret( tokenSecret );
    mAccount->setOauthConsumerKey( twitterConsumerKey );
    mAccount->setOauthConsumerSecret( twitterConsumerSecret );
    mAccount->setUsingOAuth(true);
    saveTimelinesTableState();
    mAccount->writeConfig();
    return mAccount;
}

void TwitterEditAccountWidget::authorizeUser()
{
    kDebug();
    qoauth = new QOAuth::Interface(new KIO::AccessManager(this), this);//TODO KDE 4.5 Change to use new class.
    // set the consumer key and secret
    qoauth->setConsumerKey( twitterConsumerKey );
    qoauth->setConsumerSecret( twitterConsumerSecret );
    // set a timeout for requests (in msecs)
    qoauth->setRequestTimeout( 20000 );
    qoauth->setIgnoreSslErrors(true);

    QOAuth::ParamMap otherArgs;

    // send a request for an unauthorized token
    QOAuth::ParamMap reply =
        qoauth->requestToken( "https://twitter.com/oauth/request_token",
                              QOAuth::GET, QOAuth::HMAC_SHA1 );

    // if no error occurred, read the received token and token secret
    if ( qoauth->error() == QOAuth::NoError ) {
        token = reply.value( QOAuth::tokenParameterName() );
        tokenSecret = reply.value( QOAuth::tokenSecretParameterName() );
        kDebug()<<"token: "<<token;
        QUrl url("https://twitter.com/oauth/authorize");
        url.addQueryItem("oauth_token", token);
        url.addQueryItem( "oauth_callback", "oob" );
        Choqok::openUrl(url);
        getPinCode();
    } else {
        kDebug()<<"ERROR: " <<qoauth->error()<<' '<<Choqok::qoauthErrorText(qoauth->error());
        KMessageBox::detailedError(this, i18n("Authorization Error"),
                                   Choqok::qoauthErrorText(qoauth->error()));
    }
}

void TwitterEditAccountWidget::getPinCode()
{
    isAuthenticated = false;
    while(!isAuthenticated){
        QString verifier = KInputDialog::getText( i18n("PIN"),
                                                  i18n("Enter the PIN received from Twitter:"));
        if(verifier.isEmpty())
            return;
        QOAuth::ParamMap otherArgs;
        otherArgs.insert( "oauth_verifier", verifier.toUtf8() );

        // send a request to exchange Request Token for an Access Token
        QOAuth::ParamMap reply =
            qoauth->accessToken( "https://twitter.com/oauth/access_token", QOAuth::POST, token,
                                tokenSecret, QOAuth::HMAC_SHA1, otherArgs );
        // if no error occurred, read the Access Token (and other arguments, if applicable)
        if ( qoauth->error() == QOAuth::NoError ) {
            username = reply.value( "screen_name" );
            token = reply.value( QOAuth::tokenParameterName() );
            tokenSecret = reply.value( QOAuth::tokenSecretParameterName() );
            setAuthenticated(true);
            KMessageBox::information(this, i18n("Choqok is authorized successfully."),
                                     i18n("Authorized"));
        } else {
            kDebug()<<"ERROR: "<<qoauth->error()<<' '<<Choqok::qoauthErrorText(qoauth->error());
            KMessageBox::detailedError(this, i18n("Authorization Error"),
                                    Choqok::qoauthErrorText(qoauth->error()));
        }
    }
}

void TwitterEditAccountWidget::setAuthenticated(bool authenticated)
{
    isAuthenticated = authenticated;
    if(authenticated){
        kcfg_authorize->setIcon(KIcon("object-unlocked"));
        kcfg_authenticateLed->on();
        kcfg_authenticateStatus->setText(i18n("Authenticated"));
    } else {
        kcfg_authorize->setIcon(KIcon("object-locked"));
        kcfg_authenticateLed->off();
        kcfg_authenticateStatus->setText(i18n("Not Authenticated"));
    }
}

void TwitterEditAccountWidget::loadTimelinesTableState()
{
    foreach(const QString &timeline, mAccount->microblog()->timelineNames()){
        int newRow = timelinesTable->rowCount();
        timelinesTable->insertRow(newRow);
        timelinesTable->setItem(newRow, 0, new QTableWidgetItem(timeline));

        QCheckBox *enable = new QCheckBox ( timelinesTable );
        enable->setChecked ( mAccount->timelineNames().contains(timeline) );
        timelinesTable->setCellWidget ( newRow, 1, enable );
    }
}

void TwitterEditAccountWidget::saveTimelinesTableState()
{
    QStringList timelines;
    int rowCount = timelinesTable->rowCount();
    for(int i=0; i<rowCount; ++i){
        QCheckBox *enable = qobject_cast<QCheckBox*>(timelinesTable->cellWidget(i, 1));
        if(enable && enable->isChecked())
            timelines<<timelinesTable->item(i, 0)->text();
    }
    timelines.removeDuplicates();
    mAccount->setTimelineNames(timelines);
}

#include "twittereditaccount.moc"
