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

#include "plurkeditaccountwidget.h"
#include "plurkmicroblog.h"
#include "plurkaccount.h"
#include <accountmanager.h>
#include <KDebug>
#include <klocalizedstring.h>

PlurkEditAccountWidget::PlurkEditAccountWidget(PlurkMicroBlog *microblog, PlurkAccount* account, QWidget* parent) : ChoqokEditAccountWidget(account, parent), mAccount(account)
{
    message = new QLabel(i18n("This account is an Akonadi resouce.\nPlease launch Akonadi Recourses Configuration to modify this account"), this);
    message->setWordWrap(true);
    if(mAccount) {
        username = mAccount->username();
    } else {
        QString newAccountAlias = microblog->serviceName();
	kDebug() << "Service Name: " << newAccountAlias;
        QString servName = newAccountAlias;
        int counter = 1;
        while(Choqok::AccountManager::self()->findAccount(newAccountAlias)){
            newAccountAlias = QString("%1%2").arg(servName).arg(counter);
            counter++;
        }
	setAccount( mAccount = new PlurkAccount(microblog, newAccountAlias) );
	kDebug() << "Account alias: " << newAccountAlias;
    }
}

PlurkEditAccountWidget::~PlurkEditAccountWidget()
{
}

bool PlurkEditAccountWidget::validateData()
{
        return true;
}

Choqok::Account* PlurkEditAccountWidget::apply()
{
    kDebug();
    mAccount->writeConfig();
    return mAccount;
}

#include "plurkeditaccountwidget.moc"
