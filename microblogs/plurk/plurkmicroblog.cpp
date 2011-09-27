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

#include "plurkaccount.h"
#include "plurkmicroblog.h"
#include "plurkeditaccountwidget.h"

K_PLUGIN_FACTORY( MyPluginFactory, registerPlugin < PlurkMicroBlog > (); )
K_EXPORT_PLUGIN( MyPluginFactory( "choqok_plurk" ) )

PlurkMicroBlog::PlurkMicroBlog( QObject *parent, const QList<QVariant> & args )
    : Choqok::MicroBlog( MyPluginFactory::componentData(), parent)
{
    setServiceName("Plurk");
    setServiceHomepageUrl("http://www.plurk.com/");
    setMicroBlogType( AkonadiResource );
}

PlurkMicroBlog::~PlurkMicroBlog()
{
}

Choqok::Account* PlurkMicroBlog::createNewAccount( const QString &alias )
{
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

/*
Choqok::UI::MicroBlogWidget* PlurkMicroBlog::createMicroBlogWidget( Choqok::Account *account, QWidget *parent )
{
    return NULL;
}
*/

Choqok::UI::ComposerWidget* PlurkMicroBlog::createComposerWidget( Choqok::Account *account, QWidget *parent )
{
    return NULL;
}

Choqok::UI::TimelineWidget* PlurkMicroBlog::createTimelineWidget( Choqok::Account *account, const QString &timelineName,
                                       QWidget *parent )
{
    return NULL;
}

Choqok::UI::PostWidget* PlurkMicroBlog::createPostWidget( Choqok::Account *account,
                                   const Choqok::Post &post, QWidget *parent )
{
    return NULL;
}

void PlurkMicroBlog::saveTimeline( Choqok::Account *account, const QString &timelineName,
                   const QList<Choqok::UI::PostWidget*> &timeline)
{
}

QList<Choqok::Post*> PlurkMicroBlog::loadTimeline( Choqok::Account *account, const QString &timelineName )
{
    return QList<Choqok::Post*>();
}

void PlurkMicroBlog::createPost( Choqok::Account *theAccount, Choqok::Post *post )
{
}

void PlurkMicroBlog::abortAllJobs( Choqok::Account *theAccount )
{
}

void PlurkMicroBlog::abortCreatePost( Choqok::Account *theAccount, Choqok::Post *post )
{
}

void PlurkMicroBlog::fetchPost( Choqok::Account *theAccount, Choqok::Post *post )
{
}

void PlurkMicroBlog::removePost( Choqok::Account *theAccount, Choqok::Post *post )
{
}

void PlurkMicroBlog::updateTimelines( Choqok::Account *theAccount )
{
}

QString PlurkMicroBlog::profileUrl( Choqok::Account *account, const QString &username) const
{
    return QString("");
}

QString PlurkMicroBlog::postUrl( Choqok::Account *account, const QString &username, const QString &postId) const
{
    return QString("");
}

Choqok::TimelineInfo* PlurkMicroBlog::timelineInfo( const QString &timelineName )
{
    return NULL;
}

#include "plurkmicroblog.moc"
