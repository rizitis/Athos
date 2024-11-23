/*
SPDX-FileCopyrightText: 2021 Hamed Masafi <hamed.masfi@gmail.com>

SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

#include "appdialog.h"
#include "libkommitwidgets_export.h"
#include "ui_mergeopenfilesdialog.h"

class LIBKOMMITWIDGETS_EXPORT MergeOpenFilesDialog : public AppDialog, private Ui::MergeOpenFilesDialog
{
    Q_OBJECT

public:
    explicit MergeOpenFilesDialog(QWidget *parent = nullptr);

    Q_REQUIRED_RESULT QString filePathLocal() const;
    void setFilePathLocal(const QString &newFilePathLocal);

    Q_REQUIRED_RESULT QString filePathRemote() const;
    void setFilePathRemote(const QString &newFilePathRemote);

    Q_REQUIRED_RESULT QString filePathBase() const;
    void setFilePathBase(const QString &newFilePathBase);
};
