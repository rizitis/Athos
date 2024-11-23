/*
SPDX-FileCopyrightText: 2021 Hamed Masafi <hamed.masfi@gmail.com>

SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

#include <QString>

namespace KommitTest
{
bool touch(const QString &fileName);
QString getTempPath();
}

class GitTestManager
{
public:
    GitTestManager();
    bool touch(const QString &fileName);
    bool run(const QString &cmd);

    void init();
    void add(const QString &file = QStringLiteral("."));
    void remove(const QString &file, bool cached = true);
    void commit(const QString &message);
    void addToIgnore(const QString &pattern);
    QString absoluteFilePath(const QString &file);

private:
    QString mPath;
    QString runGit(const QStringList &args);
};
