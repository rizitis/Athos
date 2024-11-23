/*
SPDX-FileCopyrightText: 2021 Hamed Masafi <hamed.masfi@gmail.com>

SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "widgetbase.h"

#include "git/gitmanager.h"

#include <QDialog>
#include <QPushButton>
#include <QVBoxLayout>

WidgetBase::WidgetBase(QWidget *parent)
    : QWidget(parent)
{
    _git = Git::Manager::instance();
    connect(_git, &Git::Manager::pathChanged, this, &WidgetBase::git_pathChanged);
}

WidgetBase::WidgetBase(Git::Manager *git, QWidget *parent)
    : QWidget(parent)
    , _git(git)
{
    if (!_git)
        _git = Git::Manager::instance();
    connect(_git, &Git::Manager::pathChanged, this, &WidgetBase::git_pathChanged);
}

Git::Manager *WidgetBase::git() const
{
    return _git;
}

void WidgetBase::setGit(Git::Manager *newGit)
{
    _git = newGit;
}

int WidgetBase::exec(QWidget *parent)
{
    auto oldParent = this->parentWidget();
    QDialog d(parent);
    QVBoxLayout layout(&d);
    this->setParent(&d);
    layout.addWidget(this);
    //    layout.addWidget(new QPushButton(&d));
    int ret = d.exec();
    setParent(oldParent);
    return ret;
}

void WidgetBase::git_pathChanged()
{
    reload();
}

#include "moc_widgetbase.cpp"
