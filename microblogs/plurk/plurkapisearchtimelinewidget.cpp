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

#include "plurkapisearchtimelinewidget.h"
#include <QHBoxLayout>
#include <KPushButton>
#include <KRestrictedLine>
#include <QCheckBox>
#include <klocalizedstring.h>
#include <KDebug>
#include <qlayoutitem.h>
#include "plurkapimicroblog.h"
#include "account.h"
#include "postwidget.h"
#include "choqoktypes.h"
#include <QLabel>
#include <choqokbehaviorsettings.h>

class PlurkApiSearchTimelineWidget::Private
{
public:
    Private(const SearchInfo &info)
        :currentPage(1), searchInfo(info), loadingAnotherPage(false)
    {}
    QPointer<KPushButton> reload;
    QPointer<KPushButton> next;
    QPointer<KPushButton> previous;
    QPointer<KRestrictedLine> pageNumber;
    QPointer<QCheckBox> autoUpdate;
    uint currentPage;
    SearchInfo searchInfo;
    QPointer<PlurkApiSearch> searchBackend;
    bool loadingAnotherPage;
};

PlurkApiSearchTimelineWidget::PlurkApiSearchTimelineWidget(Choqok::Account* account,
                                                               const QString& timelineName,
                                                               const SearchInfo &info,
                                                               QWidget* parent)
    : TimelineWidget(account, timelineName, parent), d(new Private(info))
{
    setAttribute(Qt::WA_DeleteOnClose);
    d->searchBackend = qobject_cast<PlurkApiMicroBlog*>(currentAccount()->microblog())->searchBackend();
    connect(Choqok::UI::Global::mainWindow(), SIGNAL(updateTimelines()),
            this, SLOT(slotUpdateSearchResults()) );
    if(info.isBrowsable)
        addFooter();
    timelineDescription()->setText(i18n("Search results for %1", timelineName));
    setClosable();
}

PlurkApiSearchTimelineWidget::~PlurkApiSearchTimelineWidget()
{
    delete d;
}

void PlurkApiSearchTimelineWidget::saveTimeline()
{
    kDebug();
    //There's no implementation because we don't want to have it in Search Timelines :)
//     Choqok::UI::TimelineWidget::saveTimeline();
}

void PlurkApiSearchTimelineWidget::loadTimeline()
{
    kDebug();
    //There's no implementation because we don't want to have it in Search Timelines :)
//     Choqok::UI::TimelineWidget::loadTimeline();
}

void PlurkApiSearchTimelineWidget::addFooter()
{
    QHBoxLayout *footer = titleBarLayout();
//     d->reload = new KPushButton(this);
//     d->reload->setIcon(KIcon("view-refresh"));
//     d->reload->setMaximumSize(28,28);
//     d->autoUpdate = new QCheckBox(i18n("Auto-update results"), this);
//     d->autoUpdate->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

    d->previous = new KPushButton(this);
    d->previous->setIcon(KIcon("go-previous"));
    d->previous->setMaximumSize(28,28);
    d->previous->setToolTip(i18n("Previous"));

    d->next = new KPushButton(this);
    d->next->setIcon(KIcon("go-next"));
    d->next->setMaximumSize(28,28);
    d->next->setToolTip(i18n("Next"));

    d->pageNumber = new KRestrictedLine(this);
    d->pageNumber->setValidChars("1234567890");
    d->pageNumber->setMaxLength(2);
    d->pageNumber->setMaximumWidth(40);
    d->pageNumber->setAlignment(Qt::AlignCenter);
    d->pageNumber->setToolTip( i18n("Page Number") );

//     footer->addWidget(d->reload);
//     footer->addSpacerItem(new QSpacerItem(10, 20, QSizePolicy::Expanding));
//     footer->addWidget(d->autoUpdate);
    footer->addWidget(d->previous);
    footer->addWidget(d->pageNumber);
    footer->addWidget(d->next);

//     connect( d->reload, SIGNAL(clicked(bool)), SLOT(reloadList()) );
    connect( d->next, SIGNAL(clicked(bool)), SLOT(loadNextPage()) );
    connect( d->previous, SIGNAL(clicked(bool)), SLOT(loadPreviousPage()) );
    connect( d->pageNumber, SIGNAL(returnPressed(QString)), SLOT(loadCustomPage(QString)) );

//     layout()->addItem(footer);
}

void PlurkApiSearchTimelineWidget::addNewPosts(QList< Choqok::Post* >& postList)
{
    if(d->loadingAnotherPage){
        removeAllPosts();
        d->loadingAnotherPage = false;
    }
    bool markRead = false;
    if( posts().count() < 1 )
        markRead = true;
    int m = postList.count() - Choqok::BehaviorSettings::countOfPosts();
//     kDebug()<<m<<postList.count();
    while( m > 0 ){
        postList.removeFirst();
        --m;
    }
    Choqok::UI::TimelineWidget::addNewPosts(postList);
    if(markRead)
        markAllAsRead();
    if(d->pageNumber)
        d->pageNumber->setText(QString::number(d->currentPage));
}

void PlurkApiSearchTimelineWidget::reloadList()
{
    loadCustomPage(QString::number(d->currentPage));
}

void PlurkApiSearchTimelineWidget::loadCustomPage(const QString& pageNumber)
{
    int page = pageNumber.toUInt();
    if( page == 0 )
        page = 1;
    d->loadingAnotherPage = true;
    d->currentPage = page;
    d->searchBackend->requestSearchResults(d->searchInfo, ChoqokId(), 0, page);
}

void PlurkApiSearchTimelineWidget::loadNextPage()
{
    loadCustomPage(QString::number(++d->currentPage));
}

void PlurkApiSearchTimelineWidget::loadPreviousPage()
{
    loadCustomPage(QString::number(--d->currentPage));
}

void PlurkApiSearchTimelineWidget::removeAllPosts()
{
    foreach(Choqok::UI::PostWidget *wd, posts()){
        wd->close();
    }
    posts().clear();
}

void PlurkApiSearchTimelineWidget::slotUpdateSearchResults()
{
    if(d->currentPage == 1) {
        ChoqokId lastId;
        if( !postWidgets().isEmpty() )
            lastId = postWidgets().last()->currentPost().postId;
        d->searchBackend->requestSearchResults(d->searchInfo, lastId);
    }
}

SearchInfo& PlurkApiSearchTimelineWidget::searchInfo() const
{
    return d->searchInfo;
}

#include "plurkapisearchtimelinewidget.moc"
