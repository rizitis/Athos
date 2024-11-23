/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2006 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOSHAPESIZECOMMAND_H
#define KOSHAPESIZECOMMAND_H

#include "flake_export.h"

#include <QList>
#include <kundo2command.h>

class KoShape;

/// The undo / redo command for shape sizing.
class FLAKE_EXPORT KoShapeSizeCommand : public KUndo2Command
{
public:
    /**
     * The undo / redo command for shape sizing.
     * @param shapes all the shapes that will be resized at the same time
     * @param previousSizes the old sizes; in a list with a member for each shape
     * @param newSizes the new sizes; in a list with a member for each shape
     * @param parent the parent command used for macro commands
     */
    KoShapeSizeCommand(const QList<KoShape *> &shapes, const QVector<QSizeF> &previousSizes, const QVector<QSizeF> &newSizes, KUndo2Command *parent = nullptr);
    ~KoShapeSizeCommand() override;

    /// redo the command
    void redo() override;
    /// revert the actions done in redo
    void undo() override;

private:
    class Private;
    Private *const d;
};

#endif
