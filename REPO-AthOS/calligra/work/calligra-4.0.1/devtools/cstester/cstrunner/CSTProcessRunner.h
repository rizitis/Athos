/*
 * This file is part of Calligra
 *
 * SPDX-FileCopyrightText: 2011-2012 Thorsten Zachmann <zachmann@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only
 *
 */

#ifndef CSTPROCESSRUNNER_H
#define CSTPROCESSRUNNER_H

#include <QList>
#include <QMap>
#include <QObject>
#include <QProcess>

class QString;

class CSTProcessRunner : public QObject
{
    Q_OBJECT
public:
    CSTProcessRunner(const QString &documentDir, const QString &resultDir, int concurrentProcesses, bool pickup);
    ~CSTProcessRunner() override;

public Q_SLOTS:
    void start();
    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    void startCstester(QProcess *process);
    void startMd5(QProcess *process, const QString &document);
    void logResult();

    QString m_resultDir;
    int m_concurrentProcesses;
    QMap<QProcess *, QString> m_processes;
    QList<QString> m_documents;
    QMap<int, QList<QString>> m_killed;
};

#endif /* CSTPROCESSRUNNER_H */
