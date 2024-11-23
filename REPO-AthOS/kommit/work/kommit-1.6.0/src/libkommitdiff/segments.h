/*
SPDX-FileCopyrightText: 2021 Hamed Masafi <hamed.masfi@gmail.com>

SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

#include "libkommitdiff_export.h"
#include "types.h"

#include <QStringList>

namespace Diff
{
struct LIBKOMMITDIFF_EXPORT Segment {
    virtual ~Segment() = default;

    QStringList oldText;
    QStringList newText;
    SegmentType type;

    virtual QStringList get(int index);
};

struct LIBKOMMITDIFF_EXPORT DiffSegment : Segment {
    ~DiffSegment() override = default;

    Q_REQUIRED_RESULT QStringList get(int index) override;
};

struct LIBKOMMITDIFF_EXPORT MergeSegment : Segment {
    ~MergeSegment() override = default;
    QStringList base;
    QStringList local;
    QStringList remote;
    //    MergeDiffType type;
    MergeType mergeType{None};

    Q_REQUIRED_RESULT QStringList get(int index) override;
    MergeSegment();
    MergeSegment(const QStringList &base, const QStringList &local, const QStringList &remote);
};

}
