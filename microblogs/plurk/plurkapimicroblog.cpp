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

#include "plurkapimicroblog.h"

#include <KLocale>
#include <KDebug>
#include <kio/jobclasses.h>
#include <kio/job.h>
#include <QDomElement>
#include <KAboutData>
#include <KGenericFactory>
#include <qjson/parser.h>
#include "account.h"
#include "microblogwidget.h"
#include "timelinewidget.h"
#include "editaccountwidget.h"
#include "postwidget.h"
#include "plurkapiaccount.h"
#include "plurkapipostwidget.h"
#include <KMenu>
#include <KAction>
#include <choqokuiglobal.h>
#include <accountmanager.h>
#include "plurkapidmessagedialog.h"
#include "choqokbehaviorsettings.h"
#include "choqokid.h"
#include "plurkapisearch.h"
#include "plurkapisearchdialog.h"
#include "plurkapisearchtimelinewidget.h"
#include <notifymanager.h>
#include "plurkapicomposerwidget.h"
#include <QtOAuth/QtOAuth>
#include <choqokappearancesettings.h>
#include "application.h"

class PlurkApiMicroBlog::Private
{
public:
    Private():countOfTimelinesToSave(0), friendsCursor("-1")
    {
        monthes["Jan"] = 1;
        monthes["Feb"] = 2;
        monthes["Mar"] = 3;
        monthes["Apr"] = 4;
        monthes["May"] = 5;
        monthes["Jun"] = 6;
        monthes["Jul"] = 7;
        monthes["Aug"] = 8;
        monthes["Sep"] = 9;
        monthes["Oct"] = 10;
        monthes["Nov"] = 11;
        monthes["Dec"] = 12;
    }
    int countOfTimelinesToSave;
    QString friendsCursor;
    QMap<QString, int> monthes;
    QJson::Parser parser;
    QMap< QString, QString > friendsMap;
};

PlurkApiMicroBlog::PlurkApiMicroBlog ( const KComponentData &instance, QObject *parent )
: MicroBlog( instance, parent), d(new Private)
{
    kDebug();
    KConfigGroup grp(KGlobal::config(), "PlurkApi");
    format = grp.readEntry("format", "xml");

    setCharLimit(140);
    QStringList timelineTypes;
    timelineTypes<< "All" << "Mine" << "Private" << "Responded" << "Favorite";
//    timelineTypes<< "Home" << "Reply" << "Inbox" << "Outbox" << "Favorite" << "ReTweets" << "Public";
    setTimelineNames(timelineTypes);
    timelineApiPath = "/Timeline/getPlurks";
    setTimelineInfos();
}

void PlurkApiMicroBlog::setTimelineInfos()
{
    Choqok::TimelineInfo *t = new Choqok::TimelineInfo;
    t->name = i18nc("Timeline Name", "All");
    t->description = i18nc("Timeline description", "You and your friends");
    t->icon = "user-home";
    mTimelineInfos["All"] = t;

    t = new Choqok::TimelineInfo;
    t->name = i18nc("Timeline Name", "Mine");
    t->description = i18nc("Timeline description", "Your post");
    t->icon = "edit-undo";
    mTimelineInfos["Mine"] = t;

    t = new Choqok::TimelineInfo;
    t->name = i18nc("Timeline Name", "Private");
    t->description = i18nc("Timeline description", "Your incoming private messages");
    t->icon = "mail-folder-inbox";
    mTimelineInfos["Private"] = t;

    t = new Choqok::TimelineInfo;
    t->name = i18nc("Timeline Name", "Responded");
    t->description = i18nc("Timeline description", "The messages you have replied");
    t->icon = "mail-folder-outbox";
    mTimelineInfos["Responded"] = t;

    t = new Choqok::TimelineInfo;
    t->name = i18nc("Timeline Name", "Favorite");
    t->description = i18nc("Timeline description", "Your favorites");
    t->icon = "favorites";
    mTimelineInfos["Favorite"] = t;
}

PlurkApiMicroBlog::~PlurkApiMicroBlog()
{
    kDebug();
    delete d;
}

QMenu* PlurkApiMicroBlog::createActionsMenu(Choqok::Account* theAccount, QWidget* parent)
{
    QMenu * menu = MicroBlog::createActionsMenu(theAccount, parent);

    KAction *directMessge = new KAction( KIcon("mail-message-new"), i18n("Send Private Message..."), menu );
    directMessge->setData( theAccount->alias() );
    connect( directMessge, SIGNAL(triggered(bool)), SLOT(showDirectMessageDialog()) );
    menu->addAction(directMessge);

    KAction *search = new KAction( KIcon("edit-find"), i18n("Search..."), menu );
    search->setData( theAccount->alias() );
    connect( search, SIGNAL(triggered(bool)), SLOT(showSearchDialog()) );
    menu->addAction(search);

    KAction *updateFriendsList = new KAction(KIcon("arrow-down"), i18n("Update Friends List"), menu);
    updateFriendsList->setData( theAccount->alias() );
    connect( updateFriendsList, SIGNAL(triggered(bool)), SLOT(slotUpdateFriendsList()) );
    menu->addAction(updateFriendsList);

    return menu;
}

QList< Choqok::Post* > PlurkApiMicroBlog::loadTimeline( Choqok::Account *account,
                                                          const QString& timelineName)
{
    QList< Choqok::Post* > list;
    if(timelineName.compare("Favorite") == 0)
        return list;//NOTE Won't cache favorites, and this is for compatibility with older versions!
    kDebug()<<timelineName;
    QString fileName = Choqok::AccountManager::generatePostBackupFileName(account->alias(), timelineName);
    KConfig postsBackup( "choqok/" + fileName, KConfig::NoGlobals, "data" );
    QStringList tmpList = postsBackup.groupList();

/// to don't load old archives
    if( tmpList.isEmpty() || !(QDateTime::fromString(tmpList.first()).isValid()) )
        return list;
///--------------

    QList<QDateTime> groupList;
    foreach(const QString &str, tmpList)
        groupList.append(QDateTime::fromString(str) );
    qSort(groupList);
    int count = groupList.count();
    if( count ) {
        PlurkPost *st = 0;
        for ( int i = 0; i < count; ++i ) {
            st = new PlurkPost;
            KConfigGroup grp( &postsBackup, groupList[i].toString() );
            st->postId = grp.readEntry( "plurk_id", QString() );
	    st->qualifier = grp.readEntry( "qualifier", QString() );
            st->qualifierTranslated = grp.readEntry( "qualifierTranslated", QString() );
            st->isRead = grp.readEntry( "is_unread", true);
            st->plurkType = grp.readEntry( "plurk_type", 0);
            st->author.userId = grp.readEntry( "user_id", QString() );
	    // Franklin.20110826
            st->creationDateTime = dateFromString(grp.readEntry( "posted", QDateTime::currentDateTime().toString()));
            st->noComments = grp.readEntry( "no_comments", 0 );
            st->content = grp.readEntry( "text", QString() );
            st->source = grp.readEntry( "content_raw", QString() );
            st->responseCount = grp.readEntry( "responseCount", 0 );
            st->responseSeen = grp.readEntry( "responseSeen", 0 );
            st->limitedTo= grp.readEntry( "limited_to", QString() );

            list.append( st );
        }
        mTimelineLatestId[account][timelineName] = st->postId;
    }
    return list;
}

void PlurkApiMicroBlog::saveTimeline(Choqok::Account *account,
                                       const QString& timelineName,
                                       const QList< Choqok::UI::PostWidget* > &timeline)
{
    if(timelineName.compare("Favorite") == 0)
        return;//NOTE Won't cache favorites, and this is for compatibility with older versions!
    kDebug();
    QString fileName = Choqok::AccountManager::generatePostBackupFileName(account->alias(), timelineName);
    KConfig postsBackup( "choqok/" + fileName, KConfig::NoGlobals, "data" );

    ///Clear previous data:
    QStringList prevList = postsBackup.groupList();
    int c = prevList.count();
    if ( c > 0 ) {
        for ( int i = 0; i < c; ++i ) {
            postsBackup.deleteGroup( prevList[i] );
        }
    }
    QList< Choqok::UI::PostWidget *>::const_iterator it, endIt = timeline.constEnd();
    for ( it = timeline.constBegin(); it != endIt; ++it ) {
        const Choqok::Post *post = &((*it)->currentPost());
        KConfigGroup grp( &postsBackup, post->creationDateTime.toString() );
        grp.writeEntry( "creationDateTime", post->creationDateTime );
        grp.writeEntry( "postId", post->postId.toString() );
        grp.writeEntry( "text", post->content );
        grp.writeEntry( "source", post->source );
        grp.writeEntry( "inReplyToPostId", post->replyToPostId.toString() );
        grp.writeEntry( "inReplyToUserId", post->replyToUserId.toString() );
        grp.writeEntry( "favorited", post->isFavorited );
        grp.writeEntry( "inReplyToUserName", post->replyToUserName );
        grp.writeEntry( "authorId", post->author.userId.toString() );
        grp.writeEntry( "authorUserName", post->author.userName );
        grp.writeEntry( "authorRealName", post->author.realName );
        grp.writeEntry( "authorProfileImageUrl", post->author.profileImageUrl );
        grp.writeEntry( "authorDescription" , post->author.description );
        grp.writeEntry( "isPrivate" , post->isPrivate );
        grp.writeEntry( "authorLocation" , post->author.location );
        grp.writeEntry( "isProtected" , post->author.isProtected );
        grp.writeEntry( "authorUrl" , post->author.homePageUrl );
        grp.writeEntry( "isRead" , post->isRead );
        grp.writeEntry( "repeatedFrom", post->repeatedFromUsername);
        grp.writeEntry( "repeatedPostId", post->repeatedPostId.toString());
    }
    postsBackup.sync();
    if(Choqok::Application::isShuttingDown()) {
        --d->countOfTimelinesToSave;
        if(d->countOfTimelinesToSave < 1)
            emit readyForUnload();
    }
}

Choqok::UI::ComposerWidget* PlurkApiMicroBlog::createComposerWidget(Choqok::Account* account, QWidget* parent)
{
    return new PlurkApiComposerWidget(account, parent);
}

PlurkApiSearchTimelineWidget * PlurkApiMicroBlog::createSearchTimelineWidget(Choqok::Account* theAccount,
                                                                                 QString name,
                                                                                 const SearchInfo &info,
                                                                                 QWidget* parent)
{
    return new PlurkApiSearchTimelineWidget(theAccount, name, info, parent);
}

void PlurkApiMicroBlog::createPost ( Choqok::Account* theAccount, Choqok::Post* post )
{
	kDebug();
	PlurkApiAccount* account = qobject_cast<PlurkApiAccount*>(theAccount);
	QByteArray data;
	QOAuth::ParamMap params;
	if ( !post || post->content.isEmpty() ) {
	    kDebug() << "ERROR: Status text is empty!";
	    emit errorPost ( theAccount, post, Choqok::MicroBlog::OtherError,
                         i18n ( "Creating the new post failed. Text is empty." ), MicroBlog::Critical );
	    return;
	}

	// Franklin.20110828
	KUrl url = QString("http://www.plurk.com/APP/Timeline/plurkAdd");

	// required parameters:
	data = "content=";
	data += QUrl::toPercentEncoding ( post->content );
	data += "&qualifier=";
	data += QUrl::toPercentEncoding (":");

	// optional parameters:
	//data += "&limited_to=";
	//data += "[0]";
	//data += "&no_comments=";
	//data += "1";
	//data += "&lang=";
	//data += "tr_ch";

        params.insert("content", QUrl::toPercentEncoding ( post->content ));
	params.insert("qualifier", QUrl::toPercentEncoding(":"));	// no qualifier

	// optional parameters:
	// get friends list first?
	//params.insert("limited_to", "[0]");// [0] to friends only
        //params.insert("no_comments", "1");	// 1: disable reply, 2: friends reply only
	//params.insert("lang", "tr_ch");

        KIO::StoredTransferJob *job = KIO::storedHttpPost(  data, url, KIO::HideProgressInfo ) ;
        if ( !job ) {
            kDebug() << "Cannot create an http POST request!";
            return;
        }
        job->addMetaData ( "content-type", "Content-Type: application/x-www-form-urlencoded" );
        job->addMetaData("customHTTPHeader", "Authorization: " + authorizationHeader(account, url, QOAuth::POST, params));
        mCreatePostMap[ job ] = post;
        mJobsAccount[job] = theAccount;
        connect ( job, SIGNAL ( result ( KJob* ) ), this, SLOT ( slotCreatePost ( KJob* ) ) );
        job->start();
}

void PlurkApiMicroBlog::repeatPost(Choqok::Account* theAccount, const ChoqokId& postId)
{
    kDebug();
    if ( postId.isEmpty() ) {
        kError() << "ERROR: PostId is empty!";
        return;
    }
    PlurkApiAccount* account = qobject_cast<PlurkApiAccount*>(theAccount);
    KUrl url = account->apiUrl();
    url.addPath ( QString("/statuses/retweet/%1.%2").arg(postId).arg(format) );
    QByteArray data;
    KIO::StoredTransferJob *job = KIO::storedHttpPost ( data, url, KIO::HideProgressInfo ) ;
    if ( !job ) {
        kDebug() << "Cannot create an http POST request!";
        return;
    }
    job->addMetaData ( "content-type", "Content-Type: application/x-www-form-urlencoded" );
    job->addMetaData("customHTTPHeader", "Authorization: " + authorizationHeader(account, url, QOAuth::POST));
    Choqok::Post *post = new Choqok::Post;
    post->postId = postId;
    mCreatePostMap[ job ] = post;
    mJobsAccount[job] = theAccount;
    connect ( job, SIGNAL ( result ( KJob* ) ), this, SLOT ( slotCreatePost ( KJob* ) ) );
    job->start();
}

void PlurkApiMicroBlog::slotCreatePost ( KJob *job )
{
    kDebug();
    if ( !job ) {
        kDebug() << "Job is null pointer";
        return;
    }
    Choqok::Post *post = mCreatePostMap.take(job);
    Choqok::Account *theAccount = mJobsAccount.take(job);
    if(!post || !theAccount) {
        kDebug()<<"Account or Post is NULL pointer";
        return;
    }
    if ( job->error() ) {
        kDebug() << "Job Error: " << job->errorString();
        emit errorPost ( theAccount, post, Choqok::MicroBlog::CommunicationError,
                         i18n("Creating the new post failed. %1", job->errorString()), MicroBlog::Critical );
    } else {
        KIO::StoredTransferJob *stj = qobject_cast< KIO::StoredTransferJob * > ( job );
        if ( !post->isPrivate ) {
            readPostFromJson ( theAccount, stj->data(), post );
            if ( post->isError ) {
                QString errorMsg;
                errorMsg = checkJsonForError(stj->data());
                if( errorMsg.isEmpty() ){
                    kError() << "Creating post: XML parsing error: "<< stj->data() ;
                    emit errorPost ( theAccount, post, Choqok::MicroBlog::ParsingError,
                                    i18n ( "Creating the new post failed. The result data could not be parsed." ), MicroBlog::Critical );
                } else {
                    kError() << "Server Error:" << errorMsg ;
                    emit errorPost ( theAccount, post, Choqok::MicroBlog::ServerError,
                                     i18n ( "Creating the new post failed, with error: %1", errorMsg ),
                                     MicroBlog::Critical );
                }
            } else {
                Choqok::NotifyManager::success(i18n("New post submitted successfully"));
                emit postCreated ( theAccount, post );
            }
        } else {
            Choqok::NotifyManager::success(i18n("Private message sent successfully"));
            emit postCreated ( theAccount, post );
        }
    }
}

void PlurkApiMicroBlog::abortAllJobs(Choqok::Account* theAccount)
{
    foreach ( KJob *job, mJobsAccount.keys(theAccount) ) {
        job->kill(KJob::EmitResult);
    }
}

void PlurkApiMicroBlog::abortCreatePost(Choqok::Account* theAccount, Choqok::Post *post)
{
    if( mCreatePostMap.isEmpty() )
        return;
    if( post ) {
        mCreatePostMap.key(post)->kill(KJob::EmitResult);
    } else {
        foreach( KJob *job, mCreatePostMap.keys() ){
            if(mJobsAccount[job] == theAccount)
                job->kill(KJob::EmitResult);
        }
    }
}

void PlurkApiMicroBlog::fetchPost ( Choqok::Account* theAccount, Choqok::Post* post )
{
    kDebug();
    if ( !post || post->postId.isEmpty()) {
        return;
    }
    PlurkApiAccount* account = qobject_cast<PlurkApiAccount*>(theAccount);
    KUrl url = account->apiUrl();
    url.addPath ( QString("/statuses/show/%1.%2").arg(post->postId).arg(format) );

    KIO::StoredTransferJob *job = KIO::storedGet ( url, KIO::Reload, KIO::HideProgressInfo ) ;
    if ( !job ) {
        kDebug() << "Cannot create an http GET request!";
//         QString errMsg = i18n ( "Fetching the new post failed. Cannot create an HTTP GET request."
//                                 "Please check your KDE installation." );
//         emit errorPost ( theAccount, post, Choqok::MicroBlog::OtherError, errMsg, Low );
        return;
    }
    job->addMetaData("customHTTPHeader", "Authorization: " + authorizationHeader(account, url, QOAuth::GET));
    mFetchPostMap[ job ] = post;
    mJobsAccount[ job ] = theAccount;
    connect ( job, SIGNAL ( result ( KJob* ) ), this, SLOT ( slotFetchPost ( KJob* ) ) );
    job->start();
}

void PlurkApiMicroBlog::slotFetchPost ( KJob *job )
{
    kDebug();
    if(!job) {
        kWarning()<<"NULL Job returned";
        return;
    }
    Choqok::Post *post = mFetchPostMap.take(job);
    Choqok::Account *theAccount = mJobsAccount.take(job);
    if ( job->error() ) {
        kDebug() << "Job Error: " << job->errorString();
        emit error ( theAccount, Choqok::MicroBlog::CommunicationError,
                     i18n("Fetching the new post failed. %1", job->errorString()), Low );
    } else {
        KIO::StoredTransferJob *stj = qobject_cast<KIO::StoredTransferJob *> ( job );
            readPostFromJson ( theAccount, stj->data(), post );
        if ( post->isError ) {
                QString errorMsg;
                errorMsg = checkJsonForError(stj->data());
            if( errorMsg.isEmpty() ){
                kDebug() << "Parsing Error";
                emit errorPost ( theAccount, post, Choqok::MicroBlog::ParsingError,
                                i18n ( "Fetching new post failed. The result data could not be parsed." ),
                                 Low );
            } else {
                kError()<<"Fetching post: Server Error: "<<errorMsg;
                emit errorPost ( theAccount, post, Choqok::MicroBlog::ServerError,
                                 i18n ( "Fetching new post failed, with error: %1", errorMsg ),
                                 Low );
            }
        } else {
            post->isError = true;
            emit postFetched ( theAccount, post );
            //             mFetchPostMap.remove(job);
        }
    }
}

void PlurkApiMicroBlog::removePost ( Choqok::Account* theAccount, Choqok::Post* post )
{
    kDebug();
    if ( !post->postId.isEmpty() ) {
        PlurkApiAccount* account = qobject_cast<PlurkApiAccount*>(theAccount);
        KUrl url = account->apiUrl();
        if ( !post->isPrivate ) {
            url.addPath ( "/statuses/destroy/" + post->postId + ".xml" );
        } else {
            url.addPath ( "/direct_messages/destroy/" + post->postId + ".xml" );
        }
        KIO::StoredTransferJob *job = KIO::storedHttpPost ( QByteArray(), url, KIO::HideProgressInfo ) ;
        if ( !job ) {
            kDebug() << "Cannot create an http POST request!";
//             QString errMsg = i18n ( "Removing the post failed. Cannot create an HTTP POST request. Please check your KDE installation." );
//             emit errorPost ( theAccount, post, Choqok::MicroBlog::OtherError, errMsg, MicroBlog::Critical );
            return;
        }
        job->addMetaData("customHTTPHeader", "Authorization: " + authorizationHeader(account, url, QOAuth::POST));
        mRemovePostMap[job] = post;
        mJobsAccount[job] = theAccount;
        connect ( job, SIGNAL ( result ( KJob* ) ), this, SLOT ( slotRemovePost ( KJob* ) ) );
        job->start();
    }
}

void PlurkApiMicroBlog::slotRemovePost ( KJob *job )
{
    kDebug();
    if ( !job ) {
        kDebug() << "Job is null pointer.";
        return;
    }
    Choqok::Post *post = mRemovePostMap.take(job);
    Choqok::Account *theAccount = mJobsAccount.take(job);
    if ( job->error() ) {
        kDebug() << "Job Error: " << job->errorString();
        emit errorPost ( theAccount, post, CommunicationError,
                         i18n("Removing the post failed. %1", job->errorString() ), MicroBlog::Critical );
    } else {
        KIO::StoredTransferJob *stj = qobject_cast<KIO::StoredTransferJob*>(job);
        QString errMsg = checkXmlForError(stj->data());
        if( errMsg.isEmpty() ){
            emit postRemoved ( theAccount, post );
        } else {
            kError()<<"Server error on removing post: "<<errMsg;
            emit errorPost ( theAccount, post, ServerError,
                             i18n("Removing the post failed. %1", errMsg ), MicroBlog::Critical );
        }
    }
}

void PlurkApiMicroBlog::createFavorite ( Choqok::Account* theAccount, const QString &postId )
{
    kDebug();
    PlurkApiAccount* account = qobject_cast<PlurkApiAccount*>(theAccount);
    KUrl url = account->apiUrl();
    url.addPath ( "/favorites/create/" + postId + ".xml" );
    KIO::StoredTransferJob *job = KIO::storedHttpPost ( QByteArray(), url, KIO::HideProgressInfo ) ;
    if ( !job ) {
        kDebug() << "Cannot create an http POST request!";
//         QString errMsg = i18n ( "The Favorite creation failed. Cannot create an http POST request. "
//                                 "Please check your KDE installation." );
//         emit error ( theAccount, OtherError, errMsg );
        return;
    }
    job->addMetaData("customHTTPHeader", "Authorization: " + authorizationHeader(account, url, QOAuth::POST));
    mFavoriteMap[job] = postId;
    mJobsAccount[job] = theAccount;
    connect ( job, SIGNAL ( result ( KJob* ) ), this, SLOT ( slotCreateFavorite ( KJob* ) ) );
    job->start();
}

void PlurkApiMicroBlog::slotCreateFavorite ( KJob *job )
{
    kDebug();
    if ( !job ) {
        kDebug() << "Job is null pointer.";
        return;
    }
    Choqok::Account *theAccount = mJobsAccount.take(job);
    QString postId = mFavoriteMap.take(job);
    if ( job->error() ) {
        kDebug() << "Job Error: " << job->errorString();
        emit error ( theAccount, CommunicationError, i18n( "Favorite creation failed. %1", job->errorString() ) );
    } else {
        KIO::StoredTransferJob* stJob = qobject_cast<KIO::StoredTransferJob*>( job );
        QString err = checkXmlForError(stJob->data());
        if( !err.isEmpty() ){
            emit error(theAccount, ServerError, err, Critical);
            return;
        } else {
            emit favoriteCreated ( theAccount, postId );
        }
    }
}

void PlurkApiMicroBlog::removeFavorite ( Choqok::Account* theAccount, const QString& postId )
{
    kDebug();
    PlurkApiAccount* account = qobject_cast<PlurkApiAccount*>(theAccount);
    KUrl url = account->apiUrl();
    url.addPath ( "/favorites/destroy/" + postId + ".xml" );
    KIO::StoredTransferJob *job = KIO::storedHttpPost ( QByteArray(), url, KIO::HideProgressInfo ) ;
    if ( !job ) {
        kDebug() << "Cannot create an http POST request!";
//         QString errMsg = i18n ( "Removing the favorite failed. Cannot create an http POST request. "
//                                 "Please check your KDE installation." );
//         emit error ( theAccount, OtherError, errMsg );
        return;
    }
    job->addMetaData("customHTTPHeader", "Authorization: " + authorizationHeader(account, url, QOAuth::POST));
    mFavoriteMap[job] = postId;
    mJobsAccount[job] = theAccount;
    connect ( job, SIGNAL ( result ( KJob* ) ), this, SLOT ( slotRemoveFavorite ( KJob* ) ) );
    job->start();
}

void PlurkApiMicroBlog::slotRemoveFavorite ( KJob *job )
{
    kDebug();
    if ( !job ) {
        kDebug() << "Job is null pointer.";
        return;
    }
    QString id = mFavoriteMap.take(job);
    Choqok::Account *theAccount = mJobsAccount.take(job);
    if ( job->error() ) {
        kDebug() << "Job Error: " << job->errorString();
        emit error ( theAccount, CommunicationError, i18n("Removing the favorite failed. %1", job->errorString() ) );
    } else {
        KIO::StoredTransferJob* stJob = qobject_cast<KIO::StoredTransferJob*>( job );
        QString err = checkXmlForError(stJob->data());
        if( !err.isEmpty() ){
            emit error(theAccount, ServerError, err, Critical);
            return;
        } else {
            emit favoriteRemoved ( theAccount, id );
        }
    }
}

void PlurkApiMicroBlog::getProfile(PlurkApiAccount* theAccount)
{
    KUrl url( theAccount->apiUrl() );
    url.addPath( "/Profile/getOwnProfile" );

    KIO::StoredTransferJob * job = KIO::storedGet( url, KIO::Reload, KIO::HideProgressInfo );
    if ( !job ) {
        kDebug() << "Cannot create an http GET request!";
        return;
    }

    job->addMetaData( "customHTTPHeader", "Authorization: " + authorizationHeader( theAccount, url, QOAuth::GET ) );
    mJobsAccount[job] = theAccount;
    connect( job, SIGNAL( result( KJob * ) ), this, SLOT( slotGetProfile( KJob * ) ) );
    job->start();
}

void PlurkApiMicroBlog::slotGetProfile(KJob* job)
{
    PlurkApiAccount * theAccount = qobject_cast< PlurkApiAccount * >( mJobsAccount.take( job ) );
    KIO::StoredTransferJob * stj = qobject_cast< KIO::StoredTransferJob * >( job );
    if( stj->error() ) {
        emit error(theAccount, ServerError, i18n("Profile for account %1 could not be updated:\n%2",
            theAccount->username(), stj->errorString()), Critical);
        return;
    }
    QVariantMap userData( this->readProfileFromJson( theAccount, stj->data() ) );
    theAccount->setUserId( userData["id"].toString() );
    theAccount->setUsername( userData["display_name"].toString() );
}

void PlurkApiMicroBlog::listFriendsUsername(PlurkApiAccount* theAccount)
{
    d->friendsMap.clear();
    if ( theAccount ) {
        requestFriendsScreenName(theAccount);
    }
}

void PlurkApiMicroBlog::requestFriendsScreenName(PlurkApiAccount* theAccount)
{
    KUrl url( theAccount->apiUrl() );
    url.addPath( QString("/FriendsFans/getFriendsByOffset") );

    QByteArray data;
    data += "user_id=" + QUrl::toPercentEncoding( theAccount->userId() );
    data += "&";
    data += "offset=" + QUrl::toPercentEncoding( QString::number( d->friendsMap.size() ) );
    data += "&";
    data += "limit=" + QUrl::toPercentEncoding( QString::number( 100 ) );

    QOAuth::ParamMap params;
    params.insert( "user_id", theAccount->userId().toUtf8() );
    params.insert( "offset", QString::number( d->friendsMap.size() ).toUtf8() );
    params.insert( "limit", QString::number( 100 ).toUtf8() );

    KIO::StoredTransferJob *job = KIO::storedHttpPost( data, url, KIO::HideProgressInfo ) ;
    if ( !job ) {
        kDebug() << "Cannot create an http POST request!";
        return;
    }
    job->addMetaData ( "content-type", "Content-Type: application/x-www-form-urlencoded" );
    job->addMetaData( "customHTTPHeader", "Authorization: " + authorizationHeader( theAccount, url, QOAuth::POST, params ) );
    mJobsAccount[job] = theAccount;
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotRequestFriendsScreenName(KJob*) ) );
    job->start();
    Choqok::UI::Global::mainWindow()->showStatusMessage( i18n("Updating friends list for account %1 ...", theAccount->username()) );
}

void PlurkApiMicroBlog::slotRequestFriendsScreenName(KJob* job)
{
    kDebug();
    PlurkApiAccount *theAccount = qobject_cast<PlurkApiAccount *>( mJobsAccount.take(job) );
    KIO::StoredTransferJob* stJob = qobject_cast<KIO::StoredTransferJob*>( job );
    if (stJob->error()) {
        emit error(theAccount, ServerError, i18n("Friends list for account %1 could not be updated:\n%2",
            theAccount->username(), stJob->errorString()), Critical);
        return;
    }
    QMap< QString, QString > newList;
    newList = readUsersScreenNameFromJson( theAccount, stJob->data() );
    d->friendsMap.unite( newList );
    if ( newList.count() == 100 ) {
        requestFriendsScreenName( theAccount );
    } else {
        // TODO how do we store a (id,display_name) pair?
//        theAccount->setFriendsList( friendsList );
        Choqok::UI::Global::mainWindow()->showStatusMessage(i18n("Friends list for account %1 has been updated.",
            theAccount->username()) );
        emit friendsUsernameListed( theAccount, d->friendsMap );
    }
}

void PlurkApiMicroBlog::updateTimelines (Choqok::Account* theAccount)
{
    kDebug();
    foreach ( const QString &tm, theAccount->timelineNames() ) {
        requestTimeLine ( theAccount, tm, mTimelineLatestId[theAccount][tm] );
    }
}

QString PlurkApiMicroBlog::getTimeLinePath ( QString type)
{
	const QString getPlurkUrl="/Timeline/getPlurk";		// plurk_id is required
	const QString getPlurksUrl="/Timeline/getPlurks";

	return QString("");
}

void PlurkApiMicroBlog::requestTimeLine ( Choqok::Account* theAccount, QString type,
                                            QString latestStatusId, int page, QString maxId )
{
    kDebug();
    PlurkApiAccount* account = qobject_cast<PlurkApiAccount*>(theAccount);
    KUrl url = account->apiUrl();
    url.addPath ( timelineApiPath);
    kDebug() << "requestTimeLine(): type= " << type << " url= " << url << " timelineApiPath=" << timelineApiPath;

    KUrl tmpUrl(url);

    int countOfPost = Choqok::BehaviorSettings::countOfPosts();

    QOAuth::ParamMap params;
    params.insert ( "limit", "20" );
    url.addQueryItem ( "limit", "20" );
    if( type == "Mine")
    {
        params.insert( "filter", "only_user");
        url.addQueryItem( "filter", "only_user");
    }
    else if( type == "Private")
    {
        params.insert( "filter", "only_private");
        url.addQueryItem( "filter", "only_private");
    }
    else if( type == "Responded")
    {
        params.insert( "filter", "only_responded");
        url.addQueryItem( "filter", "only_responded");
    }
    else if( type == "Favorite")
    {
        params.insert( "filter", "only_favorite");
        url.addQueryItem( "filter", "only_favorite");
    }

    kDebug() << "Latest " << type << " Id: " << latestStatusId << " apiReq: " << url;

    KIO::StoredTransferJob *job = KIO::storedGet ( url, KIO::Reload, KIO::HideProgressInfo ) ;
    if ( !job ) {
        kDebug() << "Cannot create an http GET request!";
//         QString errMsg = i18n ( "Cannot create an http GET request. Please check your KDE installation." );
//         emit error ( theAccount, OtherError, errMsg, Low );
        return;
    }
    job->addMetaData("customHTTPHeader", "Authorization: " + authorizationHeader(account, tmpUrl, QOAuth::GET, params));
    mRequestTimelineMap[job] = type;
    mJobsAccount[job] = theAccount;
    connect ( job, SIGNAL ( result ( KJob* ) ), this, SLOT ( slotRequestTimeline ( KJob* ) ) );
    job->start();
}

void PlurkApiMicroBlog::slotRequestTimeline ( KJob *job )
{
    kDebug();//TODO Add error detection for XML "checkXmlForError()" and JSON
    if ( !job ) {
        kDebug() << "Job is null pointer";
        return;
    }
    Choqok::Account *theAccount = mJobsAccount.take(job);
    if ( job->error() ) {
        kDebug() << "Job Error: " << job->errorString();
        emit error( theAccount, CommunicationError,
                    i18n("Timeline update failed, %1", job->errorString()), Low );
        return;
    }
    QString type = mRequestTimelineMap.take(job);
    kDebug() << "type is: " << type;
//    if( isValidTimeline(type) ) {
        KIO::StoredTransferJob* j = qobject_cast<KIO::StoredTransferJob*>( job );
        kDebug() << "data is: " << j->data();
        QList<Choqok::Post*> list;
        if( type=="Inbox" || type=="Outbox" ) {
            list = readDMessagesFromJson( theAccount, j->data() );
        } else {
            list = readTimelineFromJson( theAccount, j->data() );
        }
        if(!list.isEmpty()) {
            kDebug() << "Parse JSON ok";
            mTimelineLatestId[theAccount][type] = list.last()->postId;
            emit timelineDataReceived( theAccount, type, list );
        }
//    }
}

QByteArray PlurkApiMicroBlog::authorizationHeader(PlurkApiAccount* theAccount, const KUrl &requestUrl, QOAuth::HttpMethod method, QOAuth::ParamMap params)
{
    QByteArray auth;
    if(theAccount->usingOAuth()){
        auth = theAccount->oauthInterface()->createParametersString( requestUrl.url(), method, theAccount->oauthToken(), theAccount->oauthTokenSecret(), QOAuth::HMAC_SHA1, params, QOAuth::ParseForHeaderArguments );
//kDebug() << "******* auth=" << auth << endl;
    } else {
        auth = theAccount->username().toUtf8() + ':' + theAccount->password().toUtf8();
        auth = auth.toBase64().prepend( "Basic " );
    }
    return auth;
}

void PlurkApiMicroBlog::setRepeatedOfInfo(Choqok::Post* post, Choqok::Post* repeatedPost)
{
    if( Choqok::AppearanceSettings::showRetweetsInChoqokWay() ) {
        post->content = repeatedPost->content;
        post->replyToPostId = repeatedPost->replyToPostId;
        post->replyToUserId = repeatedPost->replyToUserId;
        post->replyToUserName = repeatedPost->replyToUserName;
        post->repeatedFromUsername = repeatedPost->author.userName;
        post->repeatedPostId = repeatedPost->postId;
    } else {
        post->content = repeatedPost->content;
        post->replyToPostId = repeatedPost->replyToPostId;
        post->replyToUserId = repeatedPost->replyToUserId;
        post->replyToUserName = repeatedPost->replyToUserName;
        post->repeatedFromUsername = post->author.userName;
        post->author = repeatedPost->author;
        post->repeatedPostId = repeatedPost->postId;
    }
    post->creationDateTime = repeatedPost->creationDateTime;
}

QDateTime PlurkApiMicroBlog::dateFromString ( const QString &date )
{
	// Franklin.20110826: Can not use QDateTime::fromString(), because
	// localized dayname/monthname will become Chinese under locale zh_TW.*
	// hence making the parsing misbehaved.

    char wday[10], mon[10], tz[10];
    int year, day, hours, minutes, seconds;

    //sscanf( qPrintable ( date ), "%*s %s %d %d:%d:%d %*s %d", s, &day, &hours, &minutes, &seconds, &year );
    // Franklin.20110826
    sscanf( qPrintable ( date ), "%s %d %s %d %d:%d:%d %s", wday, &day, mon, &year, &hours, &minutes, &seconds, tz);

    int month = d->monthes[mon];

    QDateTime recognized ( QDate ( year, month, day ), QTime ( hours, minutes, seconds ) );
    recognized.setTimeSpec( Qt::UTC );

    return recognized.toLocalTime();
}

void PlurkApiMicroBlog::aboutToUnload()
{
    d->countOfTimelinesToSave = 0;
    foreach(Choqok::Account* acc, Choqok::AccountManager::self()->accounts()){
        if(acc->microblog() == this){
//             acc->writeConfig();
            d->countOfTimelinesToSave += acc->timelineNames().count();
        }
    }
    emit saveTimelines();
}

void PlurkApiMicroBlog::showDirectMessageDialog( PlurkApiAccount *theAccount/* = 0*/,
                                                   const QString &toUsername/* = QString()*/ )
{
    kDebug();
    if( !theAccount ) {
        KAction *act = qobject_cast<KAction *>(sender());
        theAccount = qobject_cast<PlurkApiAccount*>(
                                    Choqok::AccountManager::self()->findAccount( act->data().toString() ) );
    }
    PlurkApiDMessageDialog *dmsg = new PlurkApiDMessageDialog(theAccount, Choqok::UI::Global::mainWindow());
    if(!toUsername.isEmpty())
        dmsg->setTo(toUsername);
    dmsg->show();
}

Choqok::TimelineInfo * PlurkApiMicroBlog::timelineInfo(const QString& timelineName)
{
    if( isValidTimeline(timelineName) )
        return mTimelineInfos.value(timelineName);
    else
        return 0;
}

void PlurkApiMicroBlog::showSearchDialog(PlurkApiAccount* theAccount)
{
    if( !theAccount ) {
        KAction *act = qobject_cast<KAction *>(sender());
        theAccount = qobject_cast<PlurkApiAccount*>(
                                    Choqok::AccountManager::self()->findAccount( act->data().toString() ) );
    }
    QPointer<PlurkApiSearchDialog> searchDlg = new PlurkApiSearchDialog( theAccount,
                                                                             Choqok::UI::Global::mainWindow() );
    searchDlg->show();
}

void PlurkApiMicroBlog::slotUpdateFriendsList()
{
    KAction *act = qobject_cast<KAction *>(sender());
    PlurkApiAccount* theAccount = qobject_cast<PlurkApiAccount*>(
                                        Choqok::AccountManager::self()->findAccount( act->data().toString() ) );
    listFriendsUsername(theAccount);
}

void PlurkApiMicroBlog::createFriendship( Choqok::Account *theAccount, const QString& username )
{
    kDebug();
    PlurkApiAccount* account = qobject_cast<PlurkApiAccount*>(theAccount);
    KUrl url = account->apiUrl();
    url.addPath( "/friendships/create/"+ username +".xml" );
    kDebug()<<url;

    KIO::StoredTransferJob *job = KIO::storedHttpPost( QByteArray(), url, KIO::HideProgressInfo) ;
    if ( !job ) {
        kError() << "Cannot create an http POST request!";
        return;
    }
    job->addMetaData("customHTTPHeader", "Authorization: " + authorizationHeader(account, url, QOAuth::POST));
    mJobsAccount[job] = theAccount;
    mFriendshipMap[ job ] = username;
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotCreateFriendship(KJob*) ) );
    job->start();
}

void PlurkApiMicroBlog::slotCreateFriendship(KJob* job)
{
    kDebug();
#if 0
    if(!job){
        kError()<<"Job is a null Pointer!";
        return;
    }
    PlurkApiAccount *theAccount = qobject_cast<PlurkApiAccount*>( mJobsAccount.take(job) );
    QString username = mFriendshipMap.take(job);
    if(job->error()){
        kDebug()<<"Job Error:"<<job->errorString();
        emit error ( theAccount, CommunicationError,
                     i18n("Creating friendship with %1 failed. %2", username, job->errorString() ) );
        return;
    }
    KIO::StoredTransferJob *stj = qobject_cast<KIO::StoredTransferJob*>(job);
    Choqok::User *user = readUserInfoFromXml(stj->data());
    if( user /*&& user->userName.compare(username, Qt::CaseInsensitive)*/ ){
        emit friendshipCreated(theAccount, username);
        Choqok::NotifyManager::success( i18n("You are now listening to %1's posts.", username) );
        theAccount->setFriendsList(QStringList());
        listFriendsUsername(theAccount);
    } else {
        QString errorMsg = checkXmlForError(stj->data());
        if( errorMsg.isEmpty() ){
            kDebug()<<"Parse Error: "<<stj->data();
            emit error( theAccount, ParsingError,
                        i18n("Creating friendship with %1 failed: the server returned invalid data.",
                             username ) );
        } else {
            kDebug()<<"Server error: "<<errorMsg;
            emit error( theAccount, ServerError,
                        i18n("Creating friendship with %1 failed: %2",
                            username, errorMsg ) );
        }
    }
#endif
}

void PlurkApiMicroBlog::destroyFriendship( Choqok::Account *theAccount, const QString& username )
{
    kDebug();
    PlurkApiAccount* account = qobject_cast<PlurkApiAccount*>(theAccount);
    KUrl url = account->apiUrl();
    url.addPath( "/friendships/destroy/" + username + ".xml" );
    kDebug()<<url;

    KIO::StoredTransferJob *job = KIO::storedHttpPost(QByteArray(), url, KIO::HideProgressInfo) ;
    if ( !job ) {
        kError() << "Cannot create an http POST request!";
        return;
    }
    job->addMetaData("customHTTPHeader", "Authorization: " + authorizationHeader(account, url, QOAuth::POST));
    mJobsAccount[job] = theAccount;
    mFriendshipMap[ job ] = username;
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotDestroyFriendship(KJob*) ) );
    job->start();
}

void PlurkApiMicroBlog::slotDestroyFriendship(KJob* job)
{
    kDebug();
#if 0
    if(!job){
        kError()<<"Job is a null Pointer!";
        return;
    }
    PlurkApiAccount *theAccount = qobject_cast<PlurkApiAccount*>( mJobsAccount.take(job) );
    QString username = mFriendshipMap.take(job);
    if(job->error()){
        kDebug()<<"Job Error:"<<job->errorString();
        emit error ( theAccount, CommunicationError,
                     i18n("Destroying friendship with %1 failed. %2", username, job->errorString() ) );
        return;
    }
    KIO::StoredTransferJob *stj = qobject_cast<KIO::StoredTransferJob*>(job);
    Choqok::User *user = readUserInfoFromXml(stj->data());
    if( user /*&& user->userName.compare( username, Qt::CaseInsensitive )*/ ){
        emit friendshipDestroyed(theAccount, username);
        Choqok::NotifyManager::success( i18n("You will not receive %1's updates.", username) );
        theAccount->setFriendsList(QStringList());
        listFriendsUsername(theAccount);
    } else {
        QString errorMsg = checkXmlForError(stj->data());
        if( errorMsg.isEmpty() ){
            kDebug()<<"Parse Error: "<<stj->data();
            emit error( theAccount, ParsingError,
                        i18n("Destroying friendship with %1 failed: the server returned invalid data.",
                            username ) );
        } else {
            kDebug()<<"Server error: "<<errorMsg;
            emit error( theAccount, ServerError,
                        i18n("Destroying friendship with %1 failed: %2",
                             username, errorMsg ) );
        }
    }
#endif
}

void PlurkApiMicroBlog::blockUser( Choqok::Account *theAccount, const QString& username )
{
    kDebug();
    PlurkApiAccount* account = qobject_cast<PlurkApiAccount*>(theAccount);
    KUrl url = account->apiUrl();
    url.addPath( "/blocks/create/"+ username +".xml" );

    KIO::StoredTransferJob *job = KIO::storedHttpPost(QByteArray(), url, KIO::HideProgressInfo) ;
    if ( !job ) {
        kError() << "Cannot create an http POST request!";
        return;
    }
    job->addMetaData("customHTTPHeader", "Authorization: " + authorizationHeader(account, url, QOAuth::POST));
    mJobsAccount[job] = theAccount;
    mFriendshipMap[ job ] = username;
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotBlockUser(KJob*) ) );
    job->start();
}

void PlurkApiMicroBlog::slotBlockUser(KJob* job)
{
    kDebug();
#if 0
    if(!job){
        kError()<<"Job is a null Pointer!";
        return;
    }
    Choqok::Account *theAccount = mJobsAccount.take(job);
    QString username = mFriendshipMap.take(job);
    if(job->error()){
        kDebug()<<"Job Error:"<<job->errorString();
        emit error ( theAccount, CommunicationError,
                     i18n("Blocking %1 failed. %2", username, job->errorString() ) );
        return;
    }
    Choqok::User *user = readUserInfoFromXml(qobject_cast<KIO::StoredTransferJob*>(job)->data());
    if( user /*&& user->userName.compare( username, Qt::CaseInsensitive )*/ ){
        emit userBlocked(theAccount, username);
        Choqok::NotifyManager::success( i18n("You will no longer be disturbed by %1.", username) );
    } else {
        kDebug()<<"Parse Error: "<<qobject_cast<KIO::StoredTransferJob*>(job)->data();
        emit error( theAccount, ParsingError,
                     i18n("Blocking %1 failed: the server returned invalid data.",
                          username ) );
    }
//     Choqok::User *user = readUserInfoFromXml(); TODO Check for failor!
#endif
}

QString PlurkApiMicroBlog::checkXmlForError(const QByteArray& buffer)
{
    QDomDocument doc;
    doc.setContent(buffer);
    QDomElement root = doc.documentElement();
    if( root.tagName() == "hash" ){
        QDomNode node = root.firstChild();
        QString errorMessage;
        QString request;
        while( !node.isNull() ){
            QDomElement elm = node.toElement();
            if(elm.tagName() == "error"){
                errorMessage = elm.text();
            } else if(elm.tagName() == "request"){
                request = elm.text();
            }
            node = node.nextSibling();
        }
        kError()<<"Error at request "<<request<<" : "<<errorMessage;
        return errorMessage;
    } else {
        return QString();
    }
}

///===================================================================

QJson::Parser* PlurkApiMicroBlog::jsonParser()
{
    return &d->parser;
}

QString PlurkApiMicroBlog::checkJsonForError(const QByteArray& buffer)
{
    bool ok;
    QVariantMap map = d->parser.parse(buffer, &ok).toMap();
    if(ok && map.contains("error")){
        kError()<<"Error at request "<<map.value("request").toString()<<" : "<<map.value("error").toString();
        return map.value("error").toString();
    }
    return QString();
}

QList< Choqok::Post* > PlurkApiMicroBlog::readTimelineFromJson(Choqok::Account* theAccount,
                                                                 const QByteArray& buffer)
{
    QList<Choqok::Post*> postList;
    bool ok;
    QVariantMap plurksMap= d->parser.parse(buffer, &ok).toMap();
    QVariantMap plurkUsersMap = plurksMap["plurk_users"].toMap();
    QVariantList plurksList = plurksMap["plurks"].toList();
    kDebug() << "map: " << plurksMap << endl;
//    kDebug() << "plurk_users: " << plurkUsersMap << endl;
//    kDebug() << "plurks(" << plurksList.count() << "): " << plurksList << endl;

    kDebug() << "parse result: " << ok << endl;
    if ( ok ) {
        QVariantList::const_iterator it = plurksList.constBegin();
        QVariantList::const_iterator endIt = plurksList.constEnd();
        for(; it != endIt; ++it){
            QVariantMap plurkMap= it->toMap();
            kDebug() << "owner_id: " << plurkMap["owner_id"].toString() << endl;
            postList.prepend(readPostFromJsonMap(theAccount, it->toMap(), new PlurkPost, plurkUsersMap[plurkMap["owner_id"].toString()].toMap()));
        }
    } else {
        QString err = checkJsonForError(buffer);
        if(err.isEmpty()){
            kError() << "JSON parsing failed.\nBuffer was: \n" << buffer;
            emit error(theAccount, ParsingError, i18n("Could not parse the data that has been received from the server."));
        } else {
            Q_EMIT error(theAccount, ServerError, err);
        }
        return postList;
    }
    return postList;
}

Choqok::Post* PlurkApiMicroBlog::readPostFromJson(Choqok::Account* theAccount,
                                                    const QByteArray& buffer,
                                                    Choqok::Post* post)
{
    bool ok;
//    QVariantMap map = d->parser.parse(buffer, &ok).toMap();
    QVariantMap plurksMap= d->parser.parse(buffer, &ok).toMap();
    QVariantMap plurkUserMap = plurksMap["plurk_users"].toMap();
    QVariantMap plurkMap = plurksMap["plurks"].toMap();

    if ( ok ) {
        return readPostFromJsonMap ( theAccount, plurkMap, (PlurkPost*)post, plurkUserMap );
    } else {
        if(!post){
            kError()<<"PlurkApiMicroBlog::readPostFromXml: post is NULL!";
            post = new Choqok::Post;
        }
        emit errorPost(theAccount, post, ParsingError, i18n("Could not parse the data that has been received from the server."));
        kError()<<"JSon parsing failed. Buffer was:"<<buffer;
        post->isError = true;
        return post;
    }
}

Choqok::Post* PlurkApiMicroBlog::readPostFromJsonMap(Choqok::Account* theAccount,
                                                       const QVariantMap& var,
                                                       PlurkPost* post, const QVariantMap& userMap)
{
    if(!post){
        kError()<<"PlurkApiMicroBlog::readPostFromJsonMap: post is NULL!";
        return 0;
    }
    kDebug() << "userMap: " << userMap << endl;
    kDebug() << "post: " << var << endl;

//    PlurkPost* _post= (PlurkPost*)post;
    post->content = var["content"].toString();

    // Franklin.20110827
    post->creationDateTime = dateFromString(var["posted"].toString());

    post->isFavorited = var["favorite"].toBool();
    post->postId = var["plurk_id"].toString();
    post->source = var["content_raw"].toString();
    post->plurkType = var["plurk_type"].toInt();
//    QVariantMap userMap = var["user"].toMap();
//    post->author.description = userMap["description"].toString();
    post->author.realName = userMap["full_name"].toString();
    post->author.userId = var["id"].toString();
    post->author.userName = userMap["display_name"].toString();
//    post->author.profileImageUrl = userMap["profile_image_url"].toString();
//    post->author.homePageUrl = userMap["statusnet_profile_url"].toString();
//    post->link = postUrl(theAccount, post->author.userName, post->postId);
//    post->isRead = post->isFavorited || (post->repeatedFromUsername.compare(theAccount->username(), Qt::CaseInsensitive) == 0);
    return (Choqok::Post*)post;
}

QList< Choqok::Post* > PlurkApiMicroBlog::readDMessagesFromJson(Choqok::Account* theAccount,
                                                                  const QByteArray& buffer)
{
    QList<Choqok::Post*> postList;
    bool ok;
    QVariantList list = d->parser.parse(buffer, &ok).toList();

    if ( ok ) {
        QVariantList::const_iterator it = list.constBegin();
        QVariantList::const_iterator endIt = list.constEnd();
        for(; it != endIt; ++it){
            postList.prepend(readDMessageFromJsonMap(theAccount, it->toMap()));
        }
    } else {
        QString err = checkJsonForError(buffer);
        if(err.isEmpty()){
            kError() << "JSON parsing failed.\nBuffer was: \n" << buffer;
            emit error(theAccount, ParsingError, i18n("Could not parse the data that has been received from the server."));
        } else {
            Q_EMIT error(theAccount, ServerError, err);
        }
        return postList;
    }
    return postList;
}

Choqok::Post* PlurkApiMicroBlog::readDMessageFromJson(Choqok::Account* theAccount,
                                                        const QByteArray& buffer)
{
    bool ok;
    QVariantMap map = d->parser.parse(buffer, &ok).toMap();

    if ( ok ) {
        return readDMessageFromJsonMap ( theAccount, map );
    } else {
        Choqok::Post *post = new Choqok::Post;
        post->isError = true;
        return post;
    }
}

Choqok::Post* PlurkApiMicroBlog::readDMessageFromJsonMap(Choqok::Account* theAccount,
                                                           const QVariantMap& var)
{
    Choqok::Post *msg = new Choqok::Post;

    msg->isPrivate = true;
    QString senderId, recipientId, timeStr, senderScreenName, recipientScreenName, senderProfileImageUrl,
    senderName, senderDescription, recipientProfileImageUrl, recipientName, recipientDescription;

    msg->creationDateTime = dateFromString ( var["created_at"].toString() );
    msg->content = var["text"].toString();
    msg->postId = var["id"].toString();;
    senderId = var["sender_id"].toString();
    recipientId = var["recipient_id"].toString();
    senderScreenName = var["sender_screen_name"].toString();
    recipientScreenName = var["recipient_screen_name"].toString();
    QVariantMap sender = var["sender"].toMap();
    senderProfileImageUrl = sender["profile_image_url"].toString();
    senderName = sender["name"].toString();
    senderDescription = sender["description"].toString();
    QVariantMap recipient = var["recipient"].toMap();
    recipientProfileImageUrl = recipient["profile_image_url"].toString();
    recipientName = recipient["name"].toString();
    recipientDescription = recipient["description"].toString();
    if ( senderScreenName.compare( theAccount->username(), Qt::CaseInsensitive) == 0 ) {
        msg->author.description = recipientDescription;
        msg->author.userName = recipientScreenName;
        msg->author.profileImageUrl = recipientProfileImageUrl;
        msg->author.realName = recipientName;
        msg->author.userId = recipientId;
        msg->replyToUserId = recipientId;
        msg->replyToUserName = recipientScreenName;
        msg->isRead = true;
    } else {
        msg->author.description = senderDescription;
        msg->author.userName = senderScreenName;
        msg->author.profileImageUrl = senderProfileImageUrl;
        msg->author.realName = senderName;
        msg->author.userId = senderId;
        msg->replyToUserId = recipientId;
        msg->replyToUserName = recipientScreenName;
    }
    return msg;
}

Choqok::User* PlurkApiMicroBlog::readUserInfoFromJson(const QByteArray& buffer)
{
    kError()<<"PlurkApiMicroBlog::readUserInfoFromJson: NOT IMPLEMENTED YET!";
    Q_UNUSED(buffer);
    return 0;
}

QMap< QString, QString > PlurkApiMicroBlog::readUsersScreenNameFromJson(Choqok::Account* theAccount,
                                                             const QByteArray& buffer)
{
    QMap< QString, QString > data;
    bool ok;
    QVariantList jsonList = d->parser.parse(buffer, &ok).toList();

    if ( ok ) {
        QVariantList::const_iterator it = jsonList.constBegin();
        QVariantList::const_iterator endIt = jsonList.constEnd();
        for(; it!=endIt; ++it){
            QVariantMap tmp( it->toMap() );
            data.insert( tmp["id"].toString(), tmp["display_name"].toString() );
        }
    } else {
        QString err = i18n( "Retrieving the friends list failed. The data returned from the server is corrupted." );
        kDebug() << "JSON parse error: the buffer is: \n" << buffer;
        emit error(theAccount, ParsingError, err, Critical);
    }
    return data;
}

Choqok::User PlurkApiMicroBlog::readUserFromJsonMap(Choqok::Account* theAccount, const QVariantMap& map)
{
    Q_UNUSED(theAccount);
    Choqok::User u;
    u.description = map["description"].toString();
    u.followersCount = map["followers_count"].toUInt();
    u.homePageUrl = map["url"].toString();
    u.isProtected = map["protected"].toBool();
    u.location = map["location"].toString();
    u.profileImageUrl = map["profile_image_url"].toString();
    u.realName = map["name"].toString();
    u.userId = map["id"].toString();
    u.userName = map["screen_name"].toString();
    return u;
}

QVariantMap PlurkApiMicroBlog::readProfileFromJson(PlurkApiAccount* theAccount, const QByteArray& buffer)
{
    bool ok = false;
    QVariantMap profile( d->parser.parse( buffer, &ok ).toMap() );
    if( !ok ) {
        QString err = i18n( "Retrieving the profile failed. The data returned from the server is corrupted." );
        kDebug() << "JSON parse error: the buffer is: \n" << buffer;
        return QVariantMap();
    }
    return profile["user_info"].toMap();
}

#include "plurkapimicroblog.moc"
