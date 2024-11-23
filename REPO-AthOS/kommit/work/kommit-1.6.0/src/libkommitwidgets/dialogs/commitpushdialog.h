/*
SPDX-FileCopyrightText: 2021 Hamed Masafi <hamed.masfi@gmail.com>

SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

#include "appdialog.h"
#include "libkommitwidgets_export.h"
#include "ui_commitpushdialog.h"

namespace Git
{
class Manager;
}
class ChangedFileActions;
class ChangedFilesModel;
class LIBKOMMITWIDGETS_EXPORT CommitPushDialog : public AppDialog, private Ui::CommitPushDialog
{
    Q_OBJECT

public:
    explicit CommitPushDialog(Git::Manager *git, QWidget *parent = nullptr);
    ~CommitPushDialog() override;

private:
    LIBKOMMITWIDGETS_NO_EXPORT void slotPushButtonCommitClicked();
    LIBKOMMITWIDGETS_NO_EXPORT void slotPushButtonPushClicked();
    LIBKOMMITWIDGETS_NO_EXPORT void slotToolButtonAddAllClicked();
    LIBKOMMITWIDGETS_NO_EXPORT void slotToolButtonAddNoneClicked();
    LIBKOMMITWIDGETS_NO_EXPORT void slotToolButtonAddIndexedClicked();
    LIBKOMMITWIDGETS_NO_EXPORT void slotToolButtonAddAddedClicked();
    LIBKOMMITWIDGETS_NO_EXPORT void slotToolButtonAddRemovedClicked();
    LIBKOMMITWIDGETS_NO_EXPORT void slotToolButtonAddModifiedClicked();
    LIBKOMMITWIDGETS_NO_EXPORT void slotListWidgetItemDoubleClicked(const QModelIndex &index);
    LIBKOMMITWIDGETS_NO_EXPORT void slotGroupBoxMakeCommitToggled(bool);
    LIBKOMMITWIDGETS_NO_EXPORT void slotListWidgetCustomContextMenuRequested(const QPoint &pos);
    LIBKOMMITWIDGETS_NO_EXPORT void checkButtonsEnable();

    enum Roles { StatusRole = Qt::UserRole + 1 };
    LIBKOMMITWIDGETS_NO_EXPORT void addFiles();
    LIBKOMMITWIDGETS_NO_EXPORT void reload();
    LIBKOMMITWIDGETS_NO_EXPORT void readConfig();
    LIBKOMMITWIDGETS_NO_EXPORT void writeConfig();
    ChangedFileActions *mActions = nullptr;
    ChangedFilesModel *const mModel;
};
