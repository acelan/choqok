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

#include "plurkpostwidget.h"
#include <plurkapimicroblog.h>
#include "plurksearch.h"
#include <KAction>
#include <KMenu>
#include <klocalizedstring.h>
#include <plurkapiwhoiswidget.h>
#include <plurkapiaccount.h>
#include <KPushButton>
#include <choqoktools.h>

const QRegExp PlurkPostWidget::mPlurkUserRegExp( "([\\s\\W]|^)@([a-z0-9_]+){1,20}", Qt::CaseInsensitive );
const QRegExp PlurkPostWidget::mPlurkTagRegExp("([\\s]|^)#([a-z0-9_\\x00c4\\x00e4\\x00d6\\x00f6\\x00dc\\x00fc\\x00df]+)", Qt::CaseInsensitive );

PlurkPostWidget::PlurkPostWidget(Choqok::Account* account, const Choqok::Post& post, QWidget* parent): PlurkApiPostWidget(account, post, parent)
{

}

void PlurkPostWidget::initUi()
{
    PlurkApiPostWidget::initUi();

    KPushButton *btn = buttons().value("btnResend");

    if(btn){
        QMenu *menu = new QMenu(btn);
        QAction *resend = new QAction(i18n("Manual ReSend"), menu);
        connect( resend, SIGNAL(triggered(bool)), SLOT(slotResendPost()) );
        QAction *repeat = new QAction(i18n("Retweet"), menu);
        repeat->setToolTip(i18n("Retweet post using API"));
        connect( repeat, SIGNAL(triggered(bool)), SLOT(repeatPost()) );
        // If person protects their acc, we will use simple adding RT before message
        if (!currentPost().author.isProtected)
            menu->addAction(repeat);
        menu->addAction(resend);
        btn->setMenu(menu);
    }
}

QString PlurkPostWidget::prepareStatus(const QString& text)
{
    QString res = PlurkApiPostWidget::prepareStatus(text);
    res.replace(mPlurkUserRegExp,"\\1@<a href='user://\\2'>\\2</a>");
    res.replace(mPlurkTagRegExp,"\\1#<a href='tag://\\2'>\\2</a>");
    return res;
}

void PlurkPostWidget::slotReplyToAll()
{
    QStringList nicks;
    nicks.append(currentPost().author.userName);
    
    QString txt = QString("@%1 ").arg(currentPost().author.userName);

    int pos = 0;
    while ((pos = mPlurkUserRegExp.indexIn(currentPost().content, pos)) != -1) {
        if (mPlurkUserRegExp.cap(2).toLower() != currentAccount()->username() && 
            mPlurkUserRegExp.cap(2).toLower() != currentPost().author.userName &&
            !nicks.contains(mPlurkUserRegExp.cap(2).toLower())){
            nicks.append(mPlurkUserRegExp.cap(2));
            txt += QString("@%1 ").arg(mPlurkUserRegExp.cap(2));
        }
        pos += mPlurkUserRegExp.matchedLength();
    }

    txt.chop(1);

    emit reply(txt, currentPost().postId);
}

void PlurkPostWidget::checkAnchor(const QUrl& url)
{
    QString scheme = url.scheme();
    PlurkApiMicroBlog* blog = qobject_cast<PlurkApiMicroBlog*>(currentAccount()->microblog());
    PlurkApiAccount *account = qobject_cast<PlurkApiAccount*>(currentAccount());
    if( scheme == "tag" ) {
        blog->searchBackend()->requestSearchResults(currentAccount(),
                                                    KUrl::fromPunycode(url.host().toUtf8()),
                                                    (int)PlurkSearch::ReferenceHashtag);
    } else if(scheme == "user") {
        KMenu menu;
        KAction * info = new KAction( KIcon("user-identity"), i18nc("Who is user", "Who is %1", url.host()),
                                      &menu );
        KAction * from = new KAction(KIcon("edit-find-user"), i18nc("Posts from user", "Posts from %1",url.host()),
                                     &menu);
        KAction * to = new KAction(KIcon("meeting-attending"), i18nc("Replies to user", "Replies to %1",
                                                                     url.host()),
                                   &menu);
        KAction *cont = new KAction(KIcon("user-properties"),i18nc("Including user name", "Including %1",
                                                                   url.host()),
                                    &menu);
        KAction * openInBrowser = new KAction(KIcon("applications-internet"),
                                              i18nc("Open profile page in browser",
                                                    "Open profile in browser"), &menu);
        from->setData(PlurkSearch::FromUser);
        to->setData(PlurkSearch::ToUser);
        cont->setData(PlurkSearch::ReferenceUser);
        menu.addAction(info);
        menu.addAction(from);
        menu.addAction(to);
        menu.addAction(cont);
        menu.addAction(openInBrowser);

        //Subscribe/UnSubscribe/Block
        bool isSubscribe = false;
        QString accountUsername = currentAccount()->username().toLower();
        QString postUsername = url.host().toLower();
        KAction *subscribe = 0, *block = 0, *replyTo = 0, *dMessage = 0;
        if(accountUsername != postUsername){
            menu.addSeparator();
            QMenu *actionsMenu = menu.addMenu(KIcon("applications-system"), i18n("Actions"));
            replyTo = new KAction(KIcon("edit-undo"), i18nc("Write a message to user attention", "Write to %1",
                                                          url.host()), actionsMenu);
            actionsMenu->addAction(replyTo);
            if( account->friendsList().contains( url.host(),
                Qt::CaseInsensitive ) ){
                dMessage = new KAction(KIcon("mail-message-new"), i18nc("Send direct message to user",
                                                                        "Send private message to %1",
                                                                        url.host()), actionsMenu);
                actionsMenu->addAction(dMessage);
                isSubscribe = false;//It's UnSubscribe
                subscribe = new KAction( KIcon("list-remove-user"),
                                         i18nc("Unfollow user",
                                               "Unfollow %1", url.host()), actionsMenu);
            } else {
                isSubscribe = true;
                subscribe = new KAction( KIcon("list-add-user"),
                                         i18nc("Follow user",
                                               "Follow %1", url.host()), actionsMenu);
            }
            block = new KAction( KIcon("dialog-cancel"),
                                 i18nc("Block user",
                                       "Block %1", url.host()), actionsMenu);
            actionsMenu->addAction(subscribe);
            actionsMenu->addAction(block);
        }

        QAction * ret = menu.exec(QCursor::pos());
        if(ret == 0)
            return;
        if(ret == info) {
            PlurkApiWhoisWidget *wd = new PlurkApiWhoisWidget(account, url.host(),  currentPost(), this);
            wd->show(QCursor::pos());
            return;
        } else if(ret == subscribe){
            if(isSubscribe) {
                blog->createFriendship(currentAccount(), url.host());
            } else {
                blog->destroyFriendship(currentAccount(), url.host());
            }
            return;
        }else if(ret == block){
            blog->blockUser(currentAccount(), url.host());
            return;
        } else if(ret == openInBrowser){
            Choqok::openUrl( QUrl( currentAccount()->microblog()->profileUrl(currentAccount(), url.host()) ) );
            return;
        } else if(ret == replyTo){
            emit reply( QString("@%1").arg(url.host()), QString() );
            return;
        } else if(ret == dMessage){
                blog->showDirectMessageDialog(account,url.host());
            return;
        }
        int type = ret->data().toInt();
        blog->searchBackend()->requestSearchResults(currentAccount(),
                                                    url.host(),
                                                    type);
    } else
        PlurkApiPostWidget::checkAnchor(url);
}

