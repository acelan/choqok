/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2010-2011 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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

#ifndef PLURKLISTDIALOG_H
#define PLURKLISTDIALOG_H

#include <KDialog>
#include "ui_plurklistdialog_base.h"
#include "plurklist.h"
#include <QListWidget>

namespace Choqok {
class Account;
}

class QListWidgetItem;
class PlurkMicroBlog;
class PlurkAccount;
class PlurkApiAccount;


class PlurkListDialog : public KDialog
{
    Q_OBJECT
public:
    explicit PlurkListDialog(PlurkApiAccount* theAccount, QWidget* parent = 0);
    virtual ~PlurkListDialog();

protected:
    virtual void slotButtonClicked(int button);

protected slots:
    void slotUsernameChanged(const QString & name);
    void loadUserLists();
    void slotLoadUserlists(Choqok::Account* theAccount, QString username, QList<Plurk::List> list);
    void slotListItemChanged(QListWidgetItem* item);

private:
    Ui::PlurkListDialogBase ui;
    PlurkAccount *account;
    PlurkMicroBlog *blog;
    QWidget *mainWidget;
    QListWidget *listWidget;
};

#endif // PLURKLISTDIALOG_H
