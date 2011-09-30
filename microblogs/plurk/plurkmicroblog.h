/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2011 AceLan Kao <acelan@acelan.idv.tw>

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

#ifndef PLURKMICROBLOG_H
#define PLURKMICROBLOG_H

#include "microblog.h" 

class PlurkMicroBlog : public Choqok::MicroBlog
{
    Q_OBJECT
  
public :
    PlurkMicroBlog(QObject * parent,/*const char *name,*/ const QVariantList& args);
    virtual ~PlurkMicroBlog();

    virtual Choqok::Account* createNewAccount( const QString &alias );
    virtual ChoqokEditAccountWidget* createEditAccountWidget(Choqok::Account* account, QWidget* parent);
    virtual Choqok::UI::MicroBlogWidget* createMicroBlogWidget( Choqok::Account *account, QWidget *parent );
    virtual Choqok::UI::ComposerWidget* createComposerWidget( Choqok::Account *account, QWidget *parent );
    virtual Choqok::UI::TimelineWidget* createTimelineWidget( Choqok::Account *account, const QString &timelineName,
                                                   QWidget *parent );
    virtual Choqok::UI::PostWidget* createPostWidget( Choqok::Account *account,
                                               const Choqok::Post &post, QWidget *parent );
    virtual void saveTimeline( Choqok::Account *account, const QString &timelineName,
                               const QList<Choqok::UI::PostWidget*> &timeline);
    virtual QList<Choqok::Post*> loadTimeline( Choqok::Account *account, const QString &timelineName );
    virtual void createPost( Choqok::Account *theAccount, Choqok::Post *post );
    virtual void abortAllJobs( Choqok::Account *theAccount );
    virtual void abortCreatePost( Choqok::Account *theAccount, Choqok::Post *post = 0 );
    virtual void fetchPost( Choqok::Account *theAccount, Choqok::Post *post );
    virtual void removePost( Choqok::Account *theAccount, Choqok::Post *post );
    virtual void updateTimelines( Choqok::Account *theAccount );
    virtual QString profileUrl( Choqok::Account *account, const QString &username) const;
    virtual QString postUrl( Choqok::Account *account, const QString &username, const QString &postId) const;
    virtual Choqok::TimelineInfo* timelineInfo( const QString &timelineName );

private:
    QMap<QString,Choqok::TimelineInfo*> timelineInfos;

};   
#endif
