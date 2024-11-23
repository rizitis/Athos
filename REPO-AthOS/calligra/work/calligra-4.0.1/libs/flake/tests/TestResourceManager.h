/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2008 Thorsten Zachmann <zachmann@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef TESTRESOURCEMANAGER_H
#define TESTRESOURCEMANAGER_H

#include <QObject>

class TestResourceManager : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void koShapeResource();
    void testUnitChanged();
};

#endif // TESTRESOURCEMANAGER_H
