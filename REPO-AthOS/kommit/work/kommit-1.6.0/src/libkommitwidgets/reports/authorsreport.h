/*
SPDX-FileCopyrightText: 2021 Hamed Masafi <hamed.masfi@gmail.com>

SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

#include "libkommitwidgets_export.h"

#include "abstractreport.h"
#include <QDateTime>
#include <QSharedPointer>

namespace Git
{
class Signature;
};

class LIBKOMMITWIDGETS_EXPORT AuthorsReport : public AbstractReport
{
    Q_OBJECT

public:
    AuthorsReport(Git::Manager *git, QObject *parent = nullptr);
    ~AuthorsReport() override;

    void reload() override;
    QString name() const override;

    int columnCount() const override;
    QStringList headerData() const override;

    bool supportChart() const override;
    QString axisXTitle() const override;
    QString axisYTitle() const override;

    int labelsAngle() const override;

private:
    struct DatesRange {
        int count = 0;
        QDateTime first;
        QDateTime last;

        void begin(const QDateTime &time);
        void increase(const QDateTime &time);
    };

    struct Author {
        QString email;
        QString name;

        DatesRange commits;
        DatesRange authoredCommits;
        DatesRange tags;
    };

    enum AuthorCreateReason {
        Commit,
        AuthoredCommit,
        Tag,
    };
    LIBKOMMITWIDGETS_NO_EXPORT Author *findOrCreate(QSharedPointer<Git::Signature> signature, AuthorCreateReason reason);

    QList<Author *> mData;
};
