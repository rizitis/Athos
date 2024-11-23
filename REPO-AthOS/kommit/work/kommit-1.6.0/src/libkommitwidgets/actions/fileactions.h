/*
SPDX-FileCopyrightText: 2021 Hamed Masafi <hamed.masfi@gmail.com>

SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

#include "abstractactions.h"
#include "libkommitwidgets_export.h"

namespace Git
{
class File;
};

class LIBKOMMITWIDGETS_EXPORT FileActions : public AbstractActions
{
    Q_OBJECT

public:
    explicit FileActions(Git::Manager *git, QWidget *parent = nullptr);
    void popup(const QPoint &pos);

    Q_REQUIRED_RESULT const QString &place() const;
    void setPlace(const QString &newPlace);

    Q_REQUIRED_RESULT const QString &filePath() const;
    void setFilePath(const QString &newFilePath);

    Q_REQUIRED_RESULT QSharedPointer<Git::File> file() const;
    void setFile(QSharedPointer<Git::File> file);

private:
    LIBKOMMITWIDGETS_NO_EXPORT void viewFile();
    LIBKOMMITWIDGETS_NO_EXPORT void openFile();
    LIBKOMMITWIDGETS_NO_EXPORT void saveAsFile();
    LIBKOMMITWIDGETS_NO_EXPORT void logFile();
    LIBKOMMITWIDGETS_NO_EXPORT void blameFile();
    LIBKOMMITWIDGETS_NO_EXPORT void search();
    LIBKOMMITWIDGETS_NO_EXPORT void openWith();
    LIBKOMMITWIDGETS_NO_EXPORT void diffWithHead();
    LIBKOMMITWIDGETS_NO_EXPORT void mergeWithHead();

    DEFINE_ACTION(actionSaveAs)

    DEFINE_ACTION(actionView)
    DEFINE_ACTION(actionOpen)
    DEFINE_ACTION(actionHistory)
    DEFINE_ACTION(actionBlame)
    DEFINE_ACTION(actionSearch)
    DEFINE_ACTION(actionOpenWith)
    DEFINE_ACTION(actionDiffWithHead)
    DEFINE_ACTION(actionMergeWithHead)

    QMenu *const mOpenWithMenu;
    QSharedPointer<Git::File> mFile;
};
