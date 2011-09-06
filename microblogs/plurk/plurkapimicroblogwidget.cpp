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

#include "plurkapimicroblogwidget.h"
#include "plurkapimicroblog.h"
#include "account.h"
#include "composerwidget.h"
#include <KTabWidget>
#include "plurkapisearchtimelinewidget.h"
#include <kicon.h>
#include <KDebug>
#include <QToolButton>
#include <klocalizedstring.h>
#include <KMessageBox>
#include <KAction>
#include <KMenu>
#include "plurkaccount.h"

class PlurkApiMicroBlogWidget::Private
{
public:
    Private(Choqok::Account *acc)
        :btnCloseSearch(0)
    {
        kDebug();
        mBlog = qobject_cast<PlurkApiMicroBlog *>(acc->microblog());
        this->account = qobject_cast<PlurkAccount *>(acc);
    }
    PlurkApiMicroBlog *mBlog;
    PlurkAccount *account;
    QToolButton *btnCloseSearch;
};

PlurkApiMicroBlogWidget::PlurkApiMicroBlogWidget(Choqok::Account* account, QWidget* parent)
    : MicroBlogWidget(account, parent), d(new Private(account))
{
    kDebug();
    connect( account, SIGNAL(modified(Choqok::Account*)),
             this, SLOT(slotAccountModified(Choqok::Account*)) );
    connect( d->mBlog->searchBackend(),
             SIGNAL(searchResultsReceived(SearchInfo,QList<Choqok::Post*>&)),
             SLOT(slotSearchResultsReceived(SearchInfo,QList<Choqok::Post*>&)) );
    connect(d->mBlog, SIGNAL(saveTimelines()), SLOT(saveSearchTimelinesState()));
    loadSearchTimelinesState();
}

void PlurkApiMicroBlogWidget::initUi()
{
    kDebug();
    Choqok::UI::MicroBlogWidget::initUi();
    connect(timelinesTabWidget(), SIGNAL(contextMenu(QWidget*,QPoint)),
            this, SLOT(slotContextMenu(QWidget*,QPoint)));
    connect(timelinesTabWidget(), SIGNAL(currentChanged(int)), SLOT(slotCurrentTimelineChanged(int)) );
    d->btnCloseSearch = new QToolButton( this );
    d->btnCloseSearch->setIcon(KIcon("tab-close"));
    d->btnCloseSearch->setAutoRaise(true);
    d->btnCloseSearch->setToolTip(i18nc("Close a timeline", "Close Timeline"));
    timelinesTabWidget()->setCornerWidget(d->btnCloseSearch, Qt::TopRightCorner);
    connect(d->btnCloseSearch, SIGNAL(clicked(bool)), SLOT(slotCloseCurrentSearch()) );
    slotCurrentTimelineChanged(timelinesTabWidget()->currentIndex());
}

PlurkApiMicroBlogWidget::~PlurkApiMicroBlogWidget()
{
    delete d;
}

void PlurkApiMicroBlogWidget::slotSearchResultsReceived(const SearchInfo &info,
                                                          QList< Choqok::Post* >& postsList)
{
    kDebug();
    if( info.account == currentAccount() ){
        kDebug()<<postsList.count();
        QString name = QString("%1%2").arg(d->mBlog->searchBackend()->optionCode(info.option)).arg(info.query);
        if(mSearchTimelines.contains(name)){
            mSearchTimelines.value(name)->addNewPosts(postsList);
        }else{
            if( postsList.isEmpty() ){
                KMessageBox::sorry(this, i18n("The search result is empty."));
            } else {
                addSearchTimelineWidgetToUi( name, info )->addNewPosts(postsList);
            }
        }
    }
}

PlurkApiSearchTimelineWidget* PlurkApiMicroBlogWidget::addSearchTimelineWidgetToUi(const QString& name,
                                                                                       const SearchInfo &info)
{
    kDebug();
    PlurkApiSearchTimelineWidget *mbw = d->mBlog->createSearchTimelineWidget(currentAccount(), name,
                                                                               info, this);
    if(mbw) {
        mbw->setObjectName(name);
        mSearchTimelines.insert(name, mbw);
        timelines().insert(name, mbw);
        timelinesTabWidget()->addTab(mbw, name);
        timelinesTabWidget()->setTabIcon(timelinesTabWidget()->indexOf(mbw), KIcon("edit-find"));
        connect( mbw, SIGNAL(updateUnreadCount(int)),
                    this, SLOT(slotUpdateUnreadCount(int)) );
        if(composer()) {
            connect( mbw, SIGNAL(forwardResendPost(QString)),
                     composer(), SLOT(setText(QString)) );
            connect( mbw, SIGNAL(forwardReply(QString,QString)),
                     composer(), SLOT(setText(QString,QString)) );
        }
        timelinesTabWidget()->setCurrentWidget(mbw);
    } else {
        kDebug()<<"Cannot Create a new TimelineWidget for timeline "<<name;
        return 0L;
    }
    if(timelinesTabWidget()->count() == 1)
        timelinesTabWidget()->setTabBarHidden(true);
    else
        timelinesTabWidget()->setTabBarHidden(false);
    return mbw;
}

void PlurkApiMicroBlogWidget::slotCurrentTimelineChanged(int index)
{
  if ( index > -1 ) {
      Choqok::UI::TimelineWidget *stw =
              qobject_cast<Choqok::UI::TimelineWidget *>(timelinesTabWidget()->widget(index));
      if(stw->isClosable())
          d->btnCloseSearch->setEnabled(true);
      else
          d->btnCloseSearch->setEnabled(false);
  }
}

void PlurkApiMicroBlogWidget::slotCloseCurrentSearch()
{
    Choqok::UI::TimelineWidget *stw =
            qobject_cast<Choqok::UI::TimelineWidget *>(timelinesTabWidget()->currentWidget());
    closeSearch(stw);
}

// void PlurkApiMicroBlogWidget::markAllAsRead()
// {
//     Choqok::UI::MicroBlogWidget::markAllAsRead();
//     foreach(PlurkApiSearchTimelineWidget *wd, mSearchTimelines) {
//         wd->markAllAsRead();
//         int tabIndex = timelinesTabWidget()->indexOf(wd);
//         if(tabIndex == -1)
//             continue;
//         timelinesTabWidget()->setTabText( tabIndex, wd->timelineName() );
//     }
// }

void PlurkApiMicroBlogWidget::slotAccountModified(Choqok::Account* account)
{
    foreach(const QString &timeline, account->microblog()->timelineNames()){
        if( account->timelineNames().contains(timeline) ){
            if( !timelines().contains(timeline) ){
                addTimelineWidgetToUi(timeline);
            }
        } else if( timelines().contains(timeline) ) {
            Choqok::UI::TimelineWidget *tm = timelines().take(timeline);
            tm->deleteLater();
        }
    }
}

void PlurkApiMicroBlogWidget::saveSearchTimelinesState()
{
    kDebug();
    int count = currentAccount()->configGroup()->readEntry("SearchCount", 0);
    int i = 0;
    while(i<count)
    {
        currentAccount()->configGroup()->deleteEntry("Search"+QString::number(i));
        ++i;
    }
    i = 0;
    foreach(PlurkApiSearchTimelineWidget* tm, mSearchTimelines){
        currentAccount()->configGroup()->writeEntry("Search"+QString::number(i), tm->searchInfo().toString());
        ++i;
    }
    currentAccount()->configGroup()->writeEntry("SearchCount", i);
}

void PlurkApiMicroBlogWidget::loadSearchTimelinesState()
{
    kDebug();
    int count = currentAccount()->configGroup()->readEntry("SearchCount", 0);
    int i = 0;
    while( i < count )
    {
        SearchInfo info;
        if( info.fromString(currentAccount()->configGroup()->readEntry("Search"+QString::number(i), QString())) ){
            qobject_cast<PlurkApiMicroBlog*>(currentAccount()->microblog())->searchBackend()->requestSearchResults(info);
        }
        ++i;
    }
}

void PlurkApiMicroBlogWidget::slotContextMenu(QWidget* w, const QPoint &pt)
{
    kDebug();
    Choqok::UI::TimelineWidget *sWidget = qobject_cast<Choqok::UI::TimelineWidget*>(w);
    if(sWidget->isClosable()){
        KMenu menu;
        KAction *ac = new KAction(KIcon("tab-close"), i18n("Close Timeline"), &menu);
        KAction *closeAll = new KAction(KIcon("tab-close"), i18n("Close All"), &menu);
        connect( closeAll, SIGNAL(triggered(Qt::MouseButtons,Qt::KeyboardModifiers)),
                 this, SLOT(closeAllSearches()) );
        menu.addAction(ac);
        menu.addAction(closeAll);
        QAction *res = menu.exec(pt);
        if(res == ac){
            closeSearch(sWidget);
        }
    }
}

void PlurkApiMicroBlogWidget::closeSearch(Choqok::UI::TimelineWidget* searchWidget)
{
    if(!searchWidget)
        return;
    searchWidget->markAllAsRead();
    PlurkApiSearchTimelineWidget *tst = qobject_cast<PlurkApiSearchTimelineWidget*>(searchWidget);
        timelinesTabWidget()->removePage(searchWidget);
    if(tst) {
        QString name = mSearchTimelines.key(tst);
        mSearchTimelines.value(name)->close();
        mSearchTimelines.remove(name);
        timelines().remove(name);
    } else {
        QStringList lst = d->account->timelineNames();
        lst.removeOne(searchWidget->timelineName());
        d->account->setTimelineNames(lst);
        d->account->writeConfig();
        timelines().remove(timelines().key(searchWidget));
        searchWidget->close();
    }
}

void PlurkApiMicroBlogWidget::closeAllSearches()
{
    foreach(PlurkApiSearchTimelineWidget* searchWidget, mSearchTimelines){
        closeSearch(searchWidget);
    }
    foreach(Choqok::UI::TimelineWidget* widget, timelines()){
        if(widget->isClosable()) {
            closeSearch(widget);
        }
    }
}

#include "plurkapimicroblogwidget.moc"
