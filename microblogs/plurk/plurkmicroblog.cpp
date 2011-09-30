/*
    This file is part of Choqok, the KDE micro-blogging client

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

#include <KGenericFactory>

#include "account.h"
#include "accountmanager.h"
#include "composerwidget.h"
#include "timelinewidget.h"
#include "editaccountwidget.h"
#include "postwidget.h"
#include "microblogwidget.h"

#include "plurkaccount.h"
#include "plurkmicroblog.h"
#include "plurkeditaccountwidget.h"

K_PLUGIN_FACTORY( MyPluginFactory, registerPlugin < PlurkMicroBlog > (); )
K_EXPORT_PLUGIN( MyPluginFactory( "choqok_plurk" ) )

PlurkMicroBlog::PlurkMicroBlog( QObject *parent, const QList<QVariant> & args )
    : Choqok::MicroBlog( MyPluginFactory::componentData(), parent)
{
    kDebug();
    setServiceName("Plurk");
    setServiceHomepageUrl("http://www.plurk.com/");
    setMicroBlogType( AkonadiResource );
    setCharLimit( 140);

    Choqok::TimelineInfo* info;
    info= new Choqok::TimelineInfo;
    info->name= i18nc("Timeline Name", "Home");
    info->description = i18nc("Timeline description", "You and your friends");
    info->icon= "user-home";
    addTimelineName("Home");
    timelineInfos["Home"]= info;

    info= new Choqok::TimelineInfo;
    info->name= i18nc("Timeline Name", "Mime");
    info->description = i18nc("Timeline description", "Your posts only");
    info->icon= "mail-folder-inbox";
    addTimelineName("Mime");
    timelineInfos["Mime"]= info;

    info= new Choqok::TimelineInfo;
    info->name= i18nc("Timeline Name", "Private");
    info->description = i18nc("Timeline description", "Private messages");
    info->icon= "mail-folder-outbox";
    addTimelineName("Private");
    timelineInfos["Private"]= info;

    info= new Choqok::TimelineInfo;
    info->name= i18nc("Timeline Name", "Responded");
    info->description = i18nc("Timeline description", "Responded posts");
    info->icon= "edit-undo";
    addTimelineName("Responded");
    timelineInfos["Responded"]= info;

    info= new Choqok::TimelineInfo;
    info->name= i18nc("Timeline Name", "Favorite");
    info->description = i18nc("Timeline description", "Your Favorites or rePlurk posts");
    info->icon= "favorites";
    addTimelineName("Favorite");
    timelineInfos["Favorite"]= info;
}

PlurkMicroBlog::~PlurkMicroBlog()
{
    kDebug();
}

Choqok::Account* PlurkMicroBlog::createNewAccount( const QString &alias )
{
    kDebug();
    PlurkAccount *acc = qobject_cast<PlurkAccount*>( Choqok::AccountManager::self()->findAccount(alias) );
    if(!acc) {
        return new PlurkAccount(this, alias);
    } else {
        return 0;
    }
}

ChoqokEditAccountWidget* PlurkMicroBlog::createEditAccountWidget(Choqok::Account* account, QWidget* parent)
{
    kDebug();
    PlurkAccount *acc = qobject_cast<PlurkAccount*>(account);
    if(acc || !account)
        return new PlurkEditAccountWidget(this, acc, parent);
    else{
        kDebug()<<"Account passed here was not a valid Plurk Account!";
        return 0L;
    }
}

Choqok::UI::MicroBlogWidget* PlurkMicroBlog::createMicroBlogWidget( Choqok::Account *account, QWidget *parent )
{
    kDebug();
    return new Choqok::UI::MicroBlogWidget(account,parent);
}

Choqok::UI::ComposerWidget* PlurkMicroBlog::createComposerWidget( Choqok::Account *account, QWidget *parent )
{
    kDebug();
    return new Choqok::UI::ComposerWidget( account, parent);
}

Choqok::UI::TimelineWidget* PlurkMicroBlog::createTimelineWidget( Choqok::Account *account, const QString &timelineName,
                                       QWidget *parent )
{
    kDebug();
    return new Choqok::UI::TimelineWidget( account, timelineName, parent);
}

Choqok::UI::PostWidget* PlurkMicroBlog::createPostWidget( Choqok::Account *account,
                                   const Choqok::Post &post, QWidget *parent )
{
    kDebug();
    return new Choqok::UI::PostWidget( account, post, parent);
}

void PlurkMicroBlog::saveTimeline( Choqok::Account *account, const QString &timelineName,
                   const QList<Choqok::UI::PostWidget*> &timeline)
{
    kDebug();
}

QList<Choqok::Post*> PlurkMicroBlog::loadTimeline( Choqok::Account *account, const QString &timelineName )
{
    kDebug();
    return QList<Choqok::Post*>();
}

void PlurkMicroBlog::createPost( Choqok::Account *theAccount, Choqok::Post *post )
{
    kDebug();
}

void PlurkMicroBlog::abortAllJobs( Choqok::Account *theAccount )
{
    kDebug();
}

void PlurkMicroBlog::abortCreatePost( Choqok::Account *theAccount, Choqok::Post *post )
{
    kDebug();
}

void PlurkMicroBlog::fetchPost( Choqok::Account *theAccount, Choqok::Post *post )
{
    kDebug();
}

void PlurkMicroBlog::removePost( Choqok::Account *theAccount, Choqok::Post *post )
{
    kDebug();
}

void PlurkMicroBlog::updateTimelines( Choqok::Account *theAccount )
{
    kDebug();
}

QString PlurkMicroBlog::profileUrl( Choqok::Account *account, const QString &username) const
{
    kDebug();
    return QString("");
}

QString PlurkMicroBlog::postUrl( Choqok::Account *account, const QString &username, const QString &postId) const
{
    kDebug();
    return QString("");
}

Choqok::TimelineInfo* PlurkMicroBlog::timelineInfo( const QString &timelineName )
{
    kDebug() << "timelineName: " << timelineName;
    if( !isValidTimeline( timelineName))
        return NULL;

    return timelineInfos[ timelineName];
}

#include "plurkmicroblog.moc"
