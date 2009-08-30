/*
This file is part of Choqok, the KDE micro-blogging client

Copyright (C) 2008-2009 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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
#ifndef LACONICAMICROBLOGPLUGIN_H
#define LACONICAMICROBLOGPLUGIN_H

#include <KUrl>
#include <twitterapihelper/twitterapimicroblog.h>

class ChoqokEditAccountWidget;
class KJob;

/**
@author Mehrdad Momeny \<mehrdad.momeny@gmail.com\>
*/
class LaconicaMicroBlog : public TwitterApiMicroBlog
{
Q_OBJECT
public:
    LaconicaMicroBlog( QObject* parent, const QVariantList& args  );
    ~LaconicaMicroBlog();

    virtual Choqok::Account *createNewAccount( const QString &alias );
    virtual ChoqokEditAccountWidget * createEditAccountWidget( Choqok::Account *account, QWidget *parent );
    virtual Choqok::UI::MicroBlogWidget * createMicroBlogWidget( Choqok::Account *account, QWidget *parent );
    virtual Choqok::UI::TimelineWidget * createTimelineWidget( Choqok::Account* account,
                                                           const QString& timelineName, QWidget* parent );
    virtual Choqok::UI::PostWidget* createPostWidget(Choqok::Account* account,
                                                  const Choqok::Post& post, QWidget* parent);
    virtual Choqok::UI::ComposerWidget* createComposerWidget(Choqok::Account* account, QWidget* parent);
    virtual QString profileUrl( Choqok::Account *account,const QString &username) const;
    virtual QString postUrl ( Choqok::Account *account, const QString &username,
                              const QString &postId ) const;

};

#endif
