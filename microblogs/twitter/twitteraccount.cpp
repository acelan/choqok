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

#include "twitteraccount.h"
#include <KDebug>
#include "twittermicroblog.h"

class TwitterAccount::Private
{
public:
    bool loadTwitpics;
};

TwitterAccount::TwitterAccount(TwitterMicroBlog* parent, const QString &alias)
    : TwitterApiAccount(parent, alias), d(new Private)
{
    setHost("twitter.com");
    d->loadTwitpics = configGroup()->readEntry("LoadTwitPics", false);
}

TwitterAccount::~TwitterAccount()
{
    delete d;
}

void TwitterAccount::writeConfig()
{
    configGroup()->writeEntry("LoadTwitPics", d->loadTwitpics);
    TwitterApiAccount::writeConfig();
}

bool TwitterAccount::isLoadTwitPics() const
{
    return d->loadTwitpics;
}

void TwitterAccount::setLoadTwitPics(bool load)
{
    d->loadTwitpics = load;
}

#include "twitteraccount.moc"
