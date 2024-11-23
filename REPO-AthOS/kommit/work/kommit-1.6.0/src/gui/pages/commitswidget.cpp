/*
SPDX-FileCopyrightText: 2021 Hamed Masafi <hamed.masfi@gmail.com>

SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "commitswidget.h"

#include "actions/branchactions.h"
#include "actions/commitactions.h"
#include "models/commitsfiltermodel.h"

#include <entities/branch.h>
#include <entities/commit.h>
#include <gitmanager.h>
#include <models/logsmodel.h>
#include <models/treemodel.h>
#include <windows/diffwindow.h>

#include <KommitSettings.h>

CommitsWidget::CommitsWidget(Git::Manager *git, AppWindow *parent)
    : WidgetBase(git, parent)
    , mRepoModel(new TreeModel(this))
{
    setupUi(this);
    init();
}

void CommitsWidget::reload()
{
    mRepoModel->clear();
    QStringList branchNames;
    auto branches = git()->branches(Git::Manager::BranchType::AllBranches);
    for (auto &branch : branches) {
        branchNames << branch->name();
        mBranchesMap.insert(branch->name(), branch);
    }
    mRepoModel->addData(branchNames);

    if (branchNames.contains(QStringLiteral("master")))
        mMainBranch = QStringLiteral("master");
    else if (branchNames.contains(QStringLiteral("main")))
        mMainBranch = QStringLiteral("main");

    setBranch(QString());
}

void CommitsWidget::saveState(QSettings &settings) const
{
    save(settings, splitter);
    save(settings, splitter_2);
    save(settings, treeViewRepo);
    save(settings, treeViewHistory);
}

void CommitsWidget::restoreState(QSettings &settings)
{
    restore(settings, splitter);
    restore(settings, splitter_2);
    restore(settings, treeViewRepo);
    restore(settings, treeViewHistory);
}

void CommitsWidget::settingsUpdated()
{
    mHistoryModel->setCalendarType(KommitSettings::calendarType());
}

void CommitsWidget::slotTreeViewRepoItemActivated(const QModelIndex &index)
{
    auto key = mRepoModel->key(index);
    if (!key.isEmpty())
        setBranch(key);
}

void CommitsWidget::slotTreeViewRepoCustomContextMenuRequested(const QPoint &pos)
{
    Q_UNUSED(pos)
    auto branchName = mRepoModel->fullPath(treeViewRepo->currentIndex());
    mActions->setBranchName(mBranchesMap.value(branchName));
    mActions->popup();
}

void CommitsWidget::init()
{
    mHistoryModel = new Git::LogsModel(mGit, this);
    mHistoryModel->setCalendarType(KommitSettings::calendarType());
    mHistoryModel->setFullDetails(true);
    mFilterModel = new CommitsFilterModel(mHistoryModel, this);

    treeViewHistory->setModel(mFilterModel);

    treeViewRepo->setModel(mRepoModel);

    mActions = new BranchActions(mGit, this);
    mCommitActions = new CommitActions(mGit, this);

    connect(treeViewRepo, &TreeView::itemActivated, this, &CommitsWidget::slotTreeViewRepoItemActivated);
    connect(treeViewRepo, &QTreeView::customContextMenuRequested, this, &CommitsWidget::slotTreeViewRepoCustomContextMenuRequested);
    connect(treeViewHistory, &TreeView::itemActivated, this, &CommitsWidget::slotTreeViewHistoryItemActivated);
    connect(treeViewHistory, &TreeView::itemActivated, this, &CommitsWidget::slotTreeViewHistoryItemActivated);
    connect(widgetCommitDetails, &CommitDetails::hashClicked, this, &CommitsWidget::slotTextBrowserHashClicked);
    connect(widgetCommitDetails, &CommitDetails::fileClicked, this, &CommitsWidget::slotTextBrowserFileClicked);
    connect(treeViewHistory, &QTreeView::customContextMenuRequested, this, &CommitsWidget::slotTreeViewHistoryCustomContextMenuRequested);
    connect(lineEditFilter, &QLineEdit::textChanged, this, &CommitsWidget::slotLineEditFilterTextChanged);
}

void CommitsWidget::slotTreeViewHistoryItemActivated(const QModelIndex &index)
{
    auto commit = mHistoryModel->fromIndex(mFilterModel->mapToSource(index));
    if (!commit)
        return;

    widgetCommitDetails->setCommit(commit.data()); // TODO: passing raw pointer
}

void CommitsWidget::slotTextBrowserHashClicked(const QString &hash)
{
    const auto index = mHistoryModel->findIndexByHash(hash);
    if (index.isValid()) {
        treeViewHistory->setCurrentIndex(index);
        slotTreeViewHistoryItemActivated(index);
    }
}

void CommitsWidget::slotTextBrowserFileClicked(const QString &file)
{
    auto commit = widgetCommitDetails->commit();

    if (!commit || commit->parents().isEmpty())
        return;

    QSharedPointer<Git::File> oldFile{new Git::File{mGit, commit->parents().constFirst(), file}};
    QSharedPointer<Git::File> newFile{new Git::File{mGit, commit->commitHash(), file}};

    auto diffWin = new DiffWindow(oldFile, newFile);
    diffWin->showModal();
}

void CommitsWidget::slotTreeViewHistoryCustomContextMenuRequested(const QPoint &pos)
{
    Q_UNUSED(pos)
    auto log = mHistoryModel->fromIndex(treeViewHistory->currentIndex());
    if (!log)
        return;
    mCommitActions->setCommit(log);

    mCommitActions->popup();
}

void CommitsWidget::setBranch(const QString &branchName)
{
    treeViewHistory->setItemDelegateForColumn(0, nullptr);
    mHistoryModel->setBranch(branchName);
    if (mHistoryModel->rowCount(QModelIndex()))
        treeViewHistory->setCurrentIndex(mHistoryModel->index(0));
}

void CommitsWidget::slotLineEditFilterTextChanged(const QString &text)
{
    mFilterModel->setFilterTerm(text);
}

#include "moc_commitswidget.cpp"
