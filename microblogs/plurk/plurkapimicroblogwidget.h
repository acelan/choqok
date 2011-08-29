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

#ifndef PLURKAPIMICROBLOGWIDGET_H
#define PLURKAPIMICROBLOGWIDGET_H

#include "microblogwidget.h"
#include "plurkapisearch.h"

class PlurkApiSearchTimelineWidget;

class CHOQOK_HELPER_EXPORT PlurkApiMicroBlogWidget : public Choqok::UI::MicroBlogWidget
{
    Q_OBJECT
public:
    explicit PlurkApiMicroBlogWidget(Choqok::Account* account, QWidget* parent = 0);
    ~PlurkApiMicroBlogWidget();
    virtual void initUi();

public Q_SLOTS:
//     virtual void markAllAsRead();
    void slotContextMenu(QWidget *w, const QPoint& pt);

protected Q_SLOTS:
    void closeAllSearches();
    void slotAccountModified(Choqok::Account *account);
    void slotCloseCurrentSearch();
    void slotCurrentTimelineChanged(int);
    virtual void saveSearchTimelinesState();
    virtual void loadSearchTimelinesState();
    virtual void slotSearchResultsReceived( const SearchInfo &info,
                                            QList<Choqok::Post*> &postsList );

protected:
    void closeSearch(Choqok::UI::TimelineWidget* searchWidget);
    QMap<QString, PlurkApiSearchTimelineWidget*> mSearchTimelines;
    PlurkApiSearchTimelineWidget* addSearchTimelineWidgetToUi(const QString& name,
                                                                const SearchInfo &info);
private:
    class Private;
    Private * const d;
};

#endif // PLURKAPIMICROBLOGWIDGET_H
