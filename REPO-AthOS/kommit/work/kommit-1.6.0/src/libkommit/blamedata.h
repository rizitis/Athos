/*
SPDX-FileCopyrightText: 2021 Hamed Masafi <hamed.masfi@gmail.com>

SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once
#include "gitloglist.h"
#include "libkommit_export.h"

#include <QSharedPointer>
#include <QString>

namespace Git
{

struct BlameDataRow {
    QString commitHash;
    QString code;

    QSharedPointer<Commit> log = nullptr; // TODO: change to QSharedPointer
};
bool operator==(const BlameDataRow &l, const BlameDataRow &r);
bool operator!=(const BlameDataRow &l, const BlameDataRow &r);

class LIBKOMMIT_EXPORT BlameData : public QList<BlameDataRow>
{
public:
    BlameData();
};

} // namespace Git
