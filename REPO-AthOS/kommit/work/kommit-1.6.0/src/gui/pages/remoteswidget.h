/*
SPDX-FileCopyrightText: 2021 Hamed Masafi <hamed.masfi@gmail.com>

SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

#include "ui_remoteswidget.h"
#include "widgetbase.h"

class RemotesActions;
namespace Git
{
class RemotesModel;
class Manager;
}
class RemotesWidget : public WidgetBase, private Ui::RemotesWidget
{
    Q_OBJECT

public:
    explicit RemotesWidget(Git::Manager *git, AppWindow *parent = nullptr);

    void saveState(QSettings &settings) const override;
    void restoreState(QSettings &settings) override;

private:
    void slotListViewItemActivated(const QModelIndex &index);
    void slotListViewCustomContextMenuRequested(const QPoint &pos);
    void init(Git::Manager *git);
    Git::RemotesModel *mModel = nullptr;
    RemotesActions *mActions = nullptr;
};
