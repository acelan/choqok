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

#include "plurkmicroblog.h"

#include <KLocale>
#include <KDebug>
#include <kio/jobclasses.h>
#include <kio/job.h>
#include <QDomElement>
#include <KAboutData>
#include <KGenericFactory>
#include "account.h"
#include "accountmanager.h"
#include "timelinewidget.h"
#include "editaccountwidget.h"
#include "plurkeditaccount.h"
#include "postwidget.h"
#include "plurkaccount.h"
#include "composerwidget.h"
#include "plurkpostwidget.h"
#include "plurkapimicroblogwidget.h"
#include "plurksearch.h"
#include "plurkapicomposerwidget.h"
#include "choqokappearancesettings.h"
#include <QMenu>
#include <KAction>
#include "plurklistdialog.h"
#include <KMessageBox>
#include <qjson/parser.h>
#include "choqoktypes.h"
#include "plurktimelinewidget.h"

K_PLUGIN_FACTORY( MyPluginFactory, registerPlugin < PlurkMicroBlog > (); )
K_EXPORT_PLUGIN( MyPluginFactory( "choqok_plurk" ) )

PlurkMicroBlog::PlurkMicroBlog ( QObject* parent, const QVariantList&  )
: PlurkApiMicroBlog(MyPluginFactory::componentData(), parent)
{
    kDebug();
    setServiceName("Plurk");
    setServiceHomepageUrl("http://www.plurk.com/");
}

PlurkMicroBlog::~PlurkMicroBlog()
{
    kDebug();
}

Choqok::Account * PlurkMicroBlog::createNewAccount( const QString &alias )
{
    PlurkAccount *acc = qobject_cast<PlurkAccount*>( Choqok::AccountManager::self()->findAccount(alias) );
    if(!acc) {
        return new PlurkAccount(this, alias);
    } else {
        return 0;
    }
}

ChoqokEditAccountWidget * PlurkMicroBlog::createEditAccountWidget( Choqok::Account *account, QWidget *parent )
{
    kDebug();
    PlurkAccount *acc = qobject_cast<PlurkAccount*>(account);
    if(acc || !account)
        return new PlurkEditAccountWidget(this, acc, parent);
    else{
        kDebug()<<"Account passed here is not a PlurkAccount!";
        return 0L;
    }
}

Choqok::UI::MicroBlogWidget * PlurkMicroBlog::createMicroBlogWidget( Choqok::Account *account, QWidget *parent )
{
    return new PlurkApiMicroBlogWidget(account, parent);
}

Choqok::UI::TimelineWidget * PlurkMicroBlog::createTimelineWidget( Choqok::Account *account,
                                                                 const QString &timelineName, QWidget *parent )
{
    return new PlurkTimelineWidget(account, timelineName, parent);
}

Choqok::UI::PostWidget* PlurkMicroBlog::createPostWidget(Choqok::Account* account,
                                                        const Choqok::Post &post, QWidget* parent)
{
    return new PlurkPostWidget(account, post, parent);
}

Choqok::UI::ComposerWidget* PlurkMicroBlog::createComposerWidget(Choqok::Account* account, QWidget* parent)
{
    return new PlurkApiComposerWidget(account, parent);
}

QString PlurkMicroBlog::profileUrl(Choqok::Account*, const QString& username) const
{
    return QString( "https://plurk.com/#!/%1" ).arg( username );
}

QString PlurkMicroBlog::postUrl(Choqok::Account*, const QString& username,
                                  const QString& postId) const
{
    return QString ( "https://plurk.com/%1/status/%2" ).arg ( username ).arg ( postId );
}

PlurkApiSearch* PlurkMicroBlog::searchBackend()
{
    if(!mSearchBackend)
        mSearchBackend = new PlurkSearch(this);
    return mSearchBackend;
}

QString PlurkMicroBlog::generateRepeatedByUserTooltip(const QString& username)
{
    if( Choqok::AppearanceSettings::showRetweetsInChoqokWay() )
        return i18n("Retweet of %1", username);
    else
        return i18n("Retweeted by %1", username);
}

QString PlurkMicroBlog::repeatQuestion()
{
    return i18n("Retweet to your followers?");
}

QMenu* PlurkMicroBlog::createActionsMenu(Choqok::Account* theAccount, QWidget* parent)
{
    QMenu *menu = PlurkApiMicroBlog::createActionsMenu(theAccount, parent);

    KAction *lists = new KAction( i18n("Add User List..."), menu );
    lists->setData( theAccount->alias() );
    connect( lists, SIGNAL(triggered(bool)), SLOT(showListDialog()) );
    menu->addAction(lists);

    return menu;
}

void PlurkMicroBlog::showListDialog(PlurkApiAccount* theAccount)
{
    if( !theAccount ) {
        KAction *act = qobject_cast<KAction *>(sender());
        theAccount = qobject_cast<PlurkApiAccount*>(
                                    Choqok::AccountManager::self()->findAccount( act->data().toString() ) );
    }
    QPointer<PlurkListDialog> listDlg = new PlurkListDialog( theAccount,
                                                                 Choqok::UI::Global::mainWindow() );
    listDlg->show();
}

void PlurkMicroBlog::fetchUserLists(PlurkAccount* theAccount, const QString& username)
{
    kDebug();
    if ( !theAccount) {
        return;
    }
    KUrl url = theAccount->apiUrl();
    url.addPath ( QString("/%1/lists.json").arg(username) );

    KIO::StoredTransferJob *job = KIO::storedGet ( url, KIO::Reload, KIO::HideProgressInfo ) ;
    if ( !job ) {
        kError() << "PlurkMicroBlog::loadUserLists: Cannot create an http GET request!";
        return;
    }
    job->addMetaData("customHTTPHeader", "Authorization: " + authorizationHeader(theAccount, url,
                                                                                 QOAuth::GET));
    mFetchUsersListMap[ job ] = username;
    mJobsAccount[ job ] = theAccount;
    connect ( job, SIGNAL ( result ( KJob* ) ), this, SLOT ( slotFetchUserLists(KJob*) ) );
    job->start();
}

void PlurkMicroBlog::slotFetchUserLists(KJob* job)
{
    kDebug();
    if(!job) {
        kWarning()<<"NULL Job returned";
        return;
    }
    QString username = mFetchUsersListMap.take(job);
    Choqok::Account *theAccount = mJobsAccount.take(job);
    if ( job->error() ) {
        kDebug() << "Job Error: " << job->errorString();
        emit error ( theAccount, Choqok::MicroBlog::CommunicationError,
                     i18n("Fetching %1's lists failed. %2", username, job->errorString()), Critical );
    } else {
        KIO::StoredTransferJob *stj = qobject_cast<KIO::StoredTransferJob *> ( job );
        QList<Plurk::List> list = readUserListsFromJson ( theAccount, stj->data() );
        if ( list.isEmpty() ) {
            QString errorMsg;
            errorMsg = checkJsonForError(stj->data());
            if( errorMsg.isEmpty() ){
                KMessageBox::information(choqokMainWindow, i18n("There's no list record for user %1", username));
            } else {
                emit error( theAccount, ServerError, errorMsg, Critical);
            }
        } else {
            emit userLists(theAccount, username, list);
        }
    }
}

void PlurkMicroBlog::addListTimeline( PlurkAccount* theAccount, const QString& username,
                                        const QString& listname )
{
    kDebug();
    QStringList tms = theAccount->timelineNames();
    QString name = QString("@%1/%2").arg(username).arg(listname);
    tms.append(name);
    addTimelineName(name);
    theAccount->setTimelineNames(tms);
    theAccount->writeConfig();
    QString url = QString("/%1/lists/%2/statuses").arg(username).arg(listname);
    updateTimelines(theAccount);
}

void PlurkMicroBlog::setListTimelines(PlurkAccount* theAccount, const QStringList& lists)
{
    kDebug()<<lists;
    QStringList tms = theAccount->timelineNames();
    foreach(const QString &name, lists){
        QString lst = name;
        lst.remove(0, 1);
        QStringList userlist = lst.split('/');

        QString username = userlist[0], listname = userlist[1];
        tms.append(name);
        addTimelineName(name);
        QString url = QString("/%1/lists/%2/statuses").arg(username).arg(listname);
    }
    tms.removeDuplicates();
    theAccount->setTimelineNames(tms);
}

Choqok::TimelineInfo* PlurkMicroBlog::timelineInfo(const QString& timelineName)
{
    if(timelineName.startsWith('@')){
        if(mListsInfo.contains(timelineName)) {
            return mListsInfo.value(timelineName);
        } else {
            Choqok::TimelineInfo *info = new Choqok::TimelineInfo;
            info->description = info->name = timelineName;
            info->icon = "format-list-unordered";
            mListsInfo.insert(timelineName, info);
            return info;
        }
    } else {
        return PlurkApiMicroBlog::timelineInfo(timelineName);
    }
}

QList< Plurk::List > PlurkMicroBlog::readUserListsFromJson(Choqok::Account* theAccount, QByteArray buffer)
{
    bool ok;
    QList<Plurk::List> plurkList;
    QVariantMap map = jsonParser()->parse(buffer, &ok).toMap();

    if ( ok && map.contains("lists") ) {
        QVariantList list = map["lists"].toList();
        QVariantList::const_iterator it = list.constBegin();
        QVariantList::const_iterator endIt = list.constEnd();
        for(; it != endIt; ++it){
            plurkList.append(readListFromJsonMap(theAccount, it->toMap()));
        }
    }
    return plurkList;
}

Plurk::List PlurkMicroBlog::readListFromJsonMap(Choqok::Account* theAccount, QVariantMap map)
{
    Plurk::List l;
    l.author = readUserFromJsonMap( theAccount, map["user"].toMap() );
    l.description = map["description"].toString();
    l.fullname = map["full_name"].toString();
    l.isFollowing = map["following"].toBool();
    l.listId = map["id"].toString();
    l.memberCount = map["member_count"].toInt();
    l.mode = (map["mode"].toString() == "public" ? Plurk::Public : Plurk::Private);
    l.name = map["name"].toString();
    l.slug = map["slug"].toString();
    l.subscriberCount = map["subscriber_count"].toInt();
    l.uri = map["uri"].toString();
    return l;
}

#include "plurkmicroblog.moc"
