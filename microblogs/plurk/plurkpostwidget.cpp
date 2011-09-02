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

#include <KAction>
#include <KMenu>
#include <KDebug>
#include <KMessageBox>
#include <KPushButton>
#include <klocalizedstring.h>
#include <mediamanager.h>
#include <textbrowser.h>
#include "choqoktools.h"
#include "choqokappearancesettings.h"
#include "plurkpostwidget.h"
#include "plurkapishowthread.h"
#include "plurkapimicroblog.h"
#include "plurksearch.h"
#include "plurkapiwhoiswidget.h"
#include "plurkapiaccount.h"

const QString protocols = "((https?|ftps?)://)";
const QString subdomains = "(([a-z0-9\\-_]{1,}\\.)?)";
const QString auth = "((([a-z0-9\\-_]{1,})((:[\\S]{1,})?)@)?)";
const QString domains = "(([a-z0-9\\-\\x0080-\\xFFFF_]){1,63}\\.)+";
const QString port = "(:(6553[0-5]|655[0-2][0-9]|65[0-4][\\d]{2}|6[0-4][\\d]{3}|[1-5][\\d]{4}|[1-9][\\d]{0,3}))";
const QString zone ("((a[cdefgilmnoqrstuwxz])|(b[abdefghijlmnorstvwyz])|(c[acdfghiklmnoruvxyz])|(d[ejkmoz])|(e[ceghrstu])|\
(f[ijkmor])|(g[abdefghilmnpqrstuwy])|(h[kmnrtu])|(i[delmnoqrst])|(j[emop])|(k[eghimnprwyz])|(l[abcikrstuvy])|\
(m[acdefghklmnopqrstuvwxyz])|(n[acefgilopruz])|(om)|(p[aefghklnrstwy])|(qa)|(r[eosuw])|(s[abcdeghijklmnortuvyz])|\
(t[cdfghjkmnoprtvwz])|(u[agksyz])|(v[aceginu])|(w[fs])|(ye)|(z[amrw])\
|(asia|com|info|net|org|biz|name|pro|aero|cat|coop|edu|jobs|mobi|museum|tel|travel|gov|int|mil|local|xxx)|(中国)|(公司)|(网络)|(صر)|(امارات)|(рф))");
const QString ip = "(25[0-5]|[2][0-4][0-9]|[0-1]?[\\d]{1,2})(\\.(25[0-5]|[2][0-4][0-9]|[0-1]?[\\d]{1,2})){3}";
const QString params = "(((\\/)[\\w:/\\?#\\[\\]@!\\$&\\(\\)\\*%\\+,;=\\._~\\x0080-\\xFFFF\\-\\|]{1,}|%[0-9a-f]{2})?)";

const QRegExp PlurkPostWidget::mPlurkUrlRegExp("((((" + protocols + "?)" + auth +
                          subdomains +
                          '(' + domains +
                          zone + "(?!(\\w))))|(" + protocols + '(' + ip + ")+))" +
                          '(' + port + "?)" + "((\\/)?)"  +
                          params + ')', Qt::CaseInsensitive);
const QRegExp PlurkPostWidget::mPlurkUserRegExp( "([\\s\\W]|^)@([a-z0-9_]+){1,20}", Qt::CaseInsensitive );
const QRegExp PlurkPostWidget::mPlurkTagRegExp("([\\s]|^)#([a-z0-9_\\x00c4\\x00e4\\x00d6\\x00f6\\x00dc\\x00fc\\x00df]+)", Qt::CaseInsensitive );

const KIcon PlurkPostWidget::unFavIcon(Choqok::MediaManager::convertToGrayScale(KIcon("rating").pixmap(16)) );

class PlurkPostWidget::Private
{
public:
    Private(Choqok::Account* account)
        :isBasePostShowed(false)
    {
        mBlog = qobject_cast<PlurkApiMicroBlog*>( account->microblog() );
    }
    KPushButton *btnFav;
    bool isBasePostShowed;
    PlurkApiMicroBlog *mBlog;
};

PlurkPostWidget::PlurkPostWidget(Choqok::Account* account, const Choqok::Post& post, QWidget* parent): PostWidget(account, post, parent), d(new Private(account))
{
    mainWidget()->document()->addResource( QTextDocument::ImageResource, QUrl("icon://thread"),
                             KIcon("go-top").pixmap(10) );

    connect(_mainWidget, SIGNAL(clicked(QMouseEvent*)), SLOT(mousePressEvent(QMouseEvent*)));
}

PlurkPostWidget::~PlurkPostWidget()
{
    delete d;
}

void PlurkPostWidget::initUi()
{
    Choqok::UI::PostWidget::initUi();

    KPushButton *btnRe = addButton( "btnReply",i18nc( "@info:tooltip", "Reply" ), "edit-undo" );
    QMenu *menu = new QMenu(btnRe);

    KAction *actRep = new KAction(KIcon("edit-undo"), i18n("Reply to %1", currentPost().author.userName), menu);
    menu->addAction(actRep);
    connect( actRep, SIGNAL(triggered(bool)), SLOT(slotReply()) );
    connect( btnRe, SIGNAL(clicked(bool)), SLOT(slotReply()) );

    KAction *actWrite = new KAction( KIcon("document-edit"), i18n("Write to %1", currentPost().author.userName),
                                     menu );
    menu->addAction(actWrite);
    connect( actWrite, SIGNAL(triggered(bool)), SLOT(slotWriteTo()) );

    if( !currentPost().isPrivate ) {
        KAction *actReplytoAll = new KAction(i18n("Reply to all"), menu);
        menu->addAction(actReplytoAll);
        connect( actReplytoAll, SIGNAL(triggered(bool)), SLOT(slotReplyToAll()) );
    }

    menu->setDefaultAction(actRep);
    btnRe->setDelayedMenu(menu);

    if( !currentPost().isPrivate ) {
        d->btnFav = addButton( "btnFavorite",i18nc( "@info:tooltip", "Favorite" ), "rating" );
        d->btnFav->setCheckable(true);
        connect( d->btnFav, SIGNAL(clicked(bool)), SLOT(setFavorite()) );
        updateFavStat();
    }

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

void PlurkPostWidget::mousePressEvent(QMouseEvent* ev)
{
    // TODO: fetch the responded message and build a linked list for it

    if(!isRead()) {
        setReadWithSignal();
    }

    QWidget::mousePressEvent(ev);
}

QString PlurkPostWidget::prepareStatus(const QString& text)
{
//    QRegExp mPlurkUrlRegExp("((http://.*) (\(.*\))(.*))");
    QString res = text; // Choqok::UI::PostWidget::prepareStatus(text);
//    res.replace( "&amp;", "&amp;amp;" );
//    res.replace( '<', "&lt;" );
//    res.replace( '>', "&gt;" );

    int pos = 0;
    while(((pos = mPlurkUrlRegExp.indexIn(res, pos)) != -1)) {
        QString link = mPlurkUrlRegExp.cap(0);
kDebug() << "text: " << res << endl;
        QString tmplink = link;
        if ( (pos - 1 > -1 && ( res.at( pos - 1 ) != '@' &&
             res.at( pos - 1 ) != '#' && res.at( pos - 1 ) != '!') &&
             // <a href="http..."></a>
             res.at( pos - 2 ) != '=') ||
             pos == 0 ) {
kDebug() << "link: (" << res.at( pos - 1 ) << ") " << link << endl;
            res.remove( pos, link.length() );
            if ( !tmplink.startsWith(QLatin1String("http"), Qt::CaseInsensitive) &&
                 !tmplink.startsWith(QLatin1String("ftp"), Qt::CaseInsensitive) )
                 tmplink.prepend("http://");
            static const QString hrefTemplate("<a href='%1' title='%1' target='_blank'>%2</a>");
            tmplink = hrefTemplate.arg( tmplink, link );
            res.insert( pos, tmplink );
        }
        pos += tmplink.length();
    }

    res.replace(mPlurkUserRegExp,"\\1@<a href='user://\\2'>\\2</a>");
    res.replace(mPlurkTagRegExp,"\\1#<a href='tag://\\2'>\\2</a>");
    return res;
}

QString PlurkPostWidget::generateSign()
{
    QString sign;
    QString profUrl = currentAccount()->microblog()->profileUrl(currentAccount(),
                                                                currentPost().author.userName);
    sign = "<b><a href='user://"+currentPost().author.userName+"' title=\"" +
    Qt::escape(currentPost().author.description) + "\">" + currentPost().author.userName +
    "</a> - </b>";
    //<img src=\"icon://web\" />
    sign += "<a href=\"" + currentPost().link +
    "\" title=\"" + currentPost().creationDateTime.toString( Qt::DefaultLocaleLongDate ) + "\">%1</a>";
    if ( currentPost().isPrivate ) {
        if( currentPost().replyToUserName.compare( currentAccount()->username(), Qt::CaseInsensitive ) == 0 ) {
            sign.prepend( "From " );
        } else {
            sign.prepend( "To " );
        }
    } else {
        if( !currentPost().source.isNull() ) {
            sign += " - ";
            if(currentPost().source == "ostatus" && !currentPost().author.homePageUrl.isEmpty()) {
                KUrl srcUrl(currentPost().author.homePageUrl);
                sign += i18n( "<a href='%1' title='Sent from %2 via OStatus'>%2</a>",
                              currentPost().author.homePageUrl,
                              srcUrl.host());
            } else {
                sign += currentPost().source;
            }
        }
        if ( !currentPost().replyToPostId.isEmpty() ) {
            QString link = currentAccount()->microblog()->postUrl( currentAccount(), currentPost().replyToUserName,
                                                                   currentPost().replyToPostId );
            QString showConMsg = i18n("Show Conversation");
            QString threadlink = "thread://" + currentPost().postId;
            sign += " - " +
            i18n("<a href='replyto://%1'>in reply to</a>&nbsp;<a href=\"%2\" title=\"%2\">%3</a>",
                currentPost().replyToPostId, link, webIconText) + ' ';
            sign += "<a title=\""+ showConMsg +"\" href=\"" + threadlink + "\"><img src=\"icon://thread\" /></a>";
        }
    }

    //ReTweet detection:
    if( !currentPost().repeatedFromUsername.isEmpty() ){
        QString retweet;
        retweet += "<br/>"
                +  d->mBlog->generateRepeatedByUserTooltip( QString("<a href='user://%1'>%2</a>").arg( currentPost().repeatedFromUsername).arg(currentPost().repeatedFromUsername) );
        sign.append(retweet);
    }
    sign.prepend("<p dir='ltr'>");
    sign.append( "</p>" );
    return sign;
}

void PlurkPostWidget::slotReply()
{
    setReadWithSignal();
    if(currentPost().isPrivate){
        PlurkApiAccount *account= qobject_cast<PlurkApiAccount*>( currentAccount() );
        d->mBlog->showDirectMessageDialog( account, currentPost().author.userName );
    } else {
        QString replyto = QString("@%1").arg(currentPost().author.userName);
        QString postId = currentPost().postId;
        if( !currentPost().repeatedFromUsername.isEmpty() ){
            replyto.prepend(QString("@%1 ").arg(currentPost().repeatedFromUsername));
            postId = currentPost().repeatedPostId;
        }
        emit reply( replyto, postId );
    }
}

void PlurkPostWidget::slotWriteTo()
{
    emit reply( QString("@%1").arg(currentPost().author.userName), QString() );
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

void PlurkPostWidget::setFavorite()
{
    setReadWithSignal();
    PlurkApiMicroBlog *mic = d->mBlog;
    if(currentPost().isFavorited){
        connect(mic, SIGNAL(favoriteRemoved(Choqok::Account*,QString)),
                this, SLOT(slotSetFavorite(Choqok::Account*,QString)) );
        mic->removeFavorite(currentAccount(), currentPost().postId);
    } else {
        connect(mic, SIGNAL(favoriteCreated(Choqok::Account*,QString)),
                   this, SLOT(slotSetFavorite(Choqok::Account*,QString)) );
        mic->createFavorite(currentAccount(), currentPost().postId);
    }
}

void PlurkPostWidget::slotSetFavorite(Choqok::Account *theAccount, const QString& postId)
{
    if(currentAccount() == theAccount && postId == currentPost().postId){
        kDebug()<<postId;
        Choqok::Post tmp = currentPost();
        tmp.isFavorited = !tmp.isFavorited;
        setCurrentPost(tmp);
        updateFavStat();
        disconnect(d->mBlog, SIGNAL(favoriteRemoved(Choqok::Account*,QString)),
                   this, SLOT(slotSetFavorite(Choqok::Account*,QString)) );
        disconnect(d->mBlog, SIGNAL(favoriteCreated(Choqok::Account*,QString)),
                   this, SLOT(slotSetFavorite(Choqok::Account*,QString)) );
    }
}

void PlurkPostWidget::updateFavStat()
{
    if(currentPost().isFavorited){
        d->btnFav->setChecked(true);
        d->btnFav->setIcon(KIcon("rating"));
    } else {
        d->btnFav->setChecked(false);
        d->btnFav->setIcon(unFavIcon);
    }
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
    } else {
        QString scheme = url.scheme();
        if( scheme == "replyto" ) {
            if(d->isBasePostShowed) {
                setContent( prepareStatus(currentPost().content).replace("<a href","<a style=\"text-decoration:none\" href",Qt::CaseInsensitive) );
                updateUi();
                d->isBasePostShowed = false;
                return;
            } else {
                connect(currentAccount()->microblog(), SIGNAL(postFetched(Choqok::Account*,Choqok::Post*)),
                        this, SLOT(slotBasePostFetched(Choqok::Account*,Choqok::Post*)) );
                Choqok::Post *ps = new Choqok::Post;
                ps->postId = url.host();
                currentAccount()->microblog()->fetchPost(currentAccount(), ps);
            }
        } else if (scheme == "thread") {
            PlurkApiShowThread *wd = new PlurkApiShowThread(currentAccount(), currentPost(), NULL);
            wd->resize(this->width(), wd->height());
            connect(wd, SIGNAL(forwardReply(QString,QString)),
                    this, SIGNAL(reply(QString,QString)));
            connect(wd, SIGNAL(forwardResendPost(QString)),
                    this, SIGNAL(resendPost(QString)));
            wd->show();
        } else {
            Choqok::UI::PostWidget::checkAnchor(url);
        }
    }
}

void PlurkPostWidget::slotBasePostFetched(Choqok::Account* theAccount, Choqok::Post* post)
{
    if(theAccount == currentAccount() && post && post->postId == currentPost().replyToPostId){
        kDebug();
        disconnect( currentAccount()->microblog(), SIGNAL(postFetched(Choqok::Account*,Choqok::Post*)),
                   this, SLOT(slotBasePostFetched(Choqok::Account*,Choqok::Post*)) );
        if(d->isBasePostShowed)
            return;
        d->isBasePostShowed = true;
        QString color;
        if( Choqok::AppearanceSettings::isCustomUi() ) {
            color = Choqok::AppearanceSettings::readForeColor().lighter().name();
        } else {
            color = this->palette().dark().color().name();
        }
        QString baseStatusText = "<p style=\"margin-top:10px; margin-bottom:10px; margin-left:20px;\
        margin-right:20px; text-indent:0px\"><span style=\" color:" + color + ";\">";
        baseStatusText += "<b><a href='user://"+ post->author.userName +"'>" +
        post->author.userName + "</a> :</b> ";

        baseStatusText += prepareStatus( post->content ) + "</p>";
        setContent( content().prepend( baseStatusText.replace("<a href","<a style=\"text-decoration:none\" href",Qt::CaseInsensitive) ) );
        updateUi();
//         delete post;
    }
}

void PlurkPostWidget::repeatPost()
{
    setReadWithSignal();
    ChoqokId postId;
    if(currentPost().repeatedPostId.isEmpty())
        postId = currentPost().postId;
    else
        postId = currentPost().repeatedPostId;
    if( KMessageBox::questionYesNo(Choqok::UI::Global::mainWindow(), d->mBlog->repeatQuestion(),
                               QString(), KStandardGuiItem::yes(), KStandardGuiItem::cancel(),
                               "dontAskRepeatConfirm") == KMessageBox::Yes )
        d->mBlog->repeatPost(currentAccount(), postId);
}


#include "plurkpostwidget.moc"
