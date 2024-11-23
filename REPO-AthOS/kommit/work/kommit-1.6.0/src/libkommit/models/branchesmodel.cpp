/*
SPDX-FileCopyrightText: 2021 Hamed Masafi <hamed.masfi@gmail.com>

SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "branchesmodel.h"
#include "entities/branch.h"
#include "gitmanager.h"

#include <KLocalizedString>

namespace Git
{

BranchesModel::BranchesModel(Manager *git, QObject *parent)
    : AbstractGitItemsModel{git, parent}
{
}

int BranchesModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return mData.size();
}

int BranchesModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 7;
}

QVariant BranchesModel::data(const QModelIndex &index, int role) const
{
    if (role != Qt::DisplayRole || !index.isValid() || index.row() < 0 || index.row() >= mData.size())
        return {};

    switch (index.column()) {
    case 0:
        return mData.at(index.row())->name();
    case 1:
        return mCompareWithRef.value(mData.at(index.row())).first;
    case 2:
        return mCompareWithRef.value(mData.at(index.row())).second;
    case 3:
        return mData.at(index.row())->isHead();
    case 4:
        return mData.at(index.row())->refName(); // mData.at(index.row())->commitsBehind;
    case 5:
        return mData.at(index.row())->upStreamName(); // mData.at(index.row())->commitsAhead;
    case 6:
        return mData.at(index.row())->remoteName();
    }

    return {};
}

QVariant BranchesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return {};

    switch (section) {
    case 0:
        return i18n("Name");
    case 1:
        return i18n("Commit(s) behind");
    case 2:
        return i18n("Commit(s) ahead");
    case 3:
        return i18n("Is ahead");
    case 4:
        return i18n("Ref");
    case 5:
        return i18n("Upstream");
    case 6:
        return i18n("Remote");
    }
    return {};
}

QSharedPointer<Branch> BranchesModel::fromIndex(const QModelIndex &index) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= mData.size())
        return nullptr;

    return mData.at(index.row());
}

QSharedPointer<Branch> BranchesModel::findByName(const QString &branchName) const
{
    auto i = std::find_if(mData.begin(), mData.end(), [&branchName](QSharedPointer<Branch> branch) {
        return branch->name() == branchName;
    });
    if (i == mData.end())
        return nullptr;

    return *i;
}

void BranchesModel::fill()
{
    mData.clear();

    mData = mGit->branches(Git::Manager::BranchType::AllBranches);
    calculateCommitStats();
}

const QString &BranchesModel::referenceBranch() const
{
    return mReferenceBranch;
}

void BranchesModel::calculateCommitStats()
{
    for (auto &b : mData) {
        const auto commitsInfo = mGit->uniqueCommitsOnBranches(mReferenceBranch, b->name());
        mCompareWithRef[b] = commitsInfo;
        //        b->commitsBehind = commitsInfo.first;
        //        b->commitsAhead = commitsInfo.second;
    }
    Q_EMIT dataChanged(index(0, 1), index(mData.size() - 1, 2));
}

void BranchesModel::calc()
{
    //    git_revwalk *walk;
    //    git_oid oid;
    //    git_sort_t sort;
    //    char buf[100+1];

    //    revwalk_parse_options(&sort, &args);

    //    git_revwalk_new(&walk,mGit->_repo);
    //    git_revwalk_sorting(walk, sort);
    //    revwalk_parse_revs(mGit->_repo, walk, &args);

    //    while (!git_revwalk_next(&oid, walk)) {
    //        git_oid_fmt(buf, &oid);
    //        //buf[GIT_OID_SHA1_HEXSIZE] = '\0';
    //        printf("%s\n", buf);
    //    }

    //    git_revwalk_free(walk);
}

void BranchesModel::setReferenceBranch(const QString &newReferenceBranch)
{
    mReferenceBranch = newReferenceBranch;

    calculateCommitStats();
}

const QString &BranchesModel::currentBranch() const
{
    return mCurrentBranch;
}

} // namespace Git

#include "moc_branchesmodel.cpp"
