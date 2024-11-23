/*
SPDX-FileCopyrightText: 2021 Hamed Masafi <hamed.masfi@gmail.com>

SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "switchbranchdialog.h"
#include "commands/commandswitchbranch.h"
#include "core/kmessageboxhelper.h"
#include "gitmanager.h"
#include "libkommitwidgets_appdebug.h"

#include <QDebug>

#include <KLocalizedString>

#define BRANCH_TYPE_LOCAL 1
#define BRANCH_TYPE_REMOTE 2

SwitchBranchDialog::SwitchBranchDialog(Git::Manager *git, QWidget *parent)
    : AppDialog(git, parent)
{
    setupUi(this);

    mExistingLocalBranches = git->branchesNames(Git::Manager::BranchType::LocalBranch);
    for (const auto &b : std::as_const(mExistingLocalBranches))
        comboBoxBranchSelect->addItem(b, BRANCH_TYPE_LOCAL);
    mExistingRemoteBranches = git->branchesNames(Git::Manager::BranchType::RemoteBranch);
    for (const auto &b : std::as_const(mExistingRemoteBranches))
        comboBoxBranchSelect->addItem(b, BRANCH_TYPE_REMOTE);

    comboBoxTags->addItems(git->tagsNames());
    // TODO: comboBoxTags->setModel(git->tagsModel());

    radioButtonTag->setEnabled(comboBoxTags->count());

    connect(buttonBox, &QDialogButtonBox::accepted, this, &SwitchBranchDialog::slotButtonBoxAccepted);
}

Git::CommandSwitchBranch *SwitchBranchDialog::command() const
{
    auto cmd = new Git::CommandSwitchBranch(mGit);

    if (radioButtonExistingBranch->isChecked()) {
        if (comboBoxBranchSelect->currentData().toInt() == BRANCH_TYPE_LOCAL) {
            cmd->setTarget(comboBoxBranchSelect->currentText());
            cmd->setMode(Git::CommandSwitchBranch::ExistingBranch);
        } else {
            const auto n = comboBoxBranchSelect->currentText().indexOf(QLatin1Char('/'));
            if (n == -1) {
                qCWarning(KOMMIT_WIDGETS_LOG, "The branch name %s is invalid", qPrintable(comboBoxBranchSelect->currentText()));
                cmd->deleteLater();
                return nullptr;
            }

            cmd->setRemoteBranch(comboBoxBranchSelect->currentText().mid(0, n));
            cmd->setTarget(comboBoxBranchSelect->currentText().mid(n + 1));
            cmd->setMode(Git::CommandSwitchBranch::RemoteBranch);
        }
    } else if (radioButtonTag->isChecked()) {
        cmd->setMode(Git::CommandSwitchBranch::Tag);
        cmd->setTarget(comboBoxTags->currentText());
    } else {
        cmd->setMode(Git::CommandSwitchBranch::NewBranch);
        cmd->setTarget(lineEditNewBranchName->text());
    }

    return cmd;
}

void SwitchBranchDialog::slotButtonBoxAccepted()
{
    if (radioButtonExistingBranch->isChecked())
        if (mExistingLocalBranches.contains(lineEditNewBranchName->text()) || mExistingRemoteBranches.contains(lineEditNewBranchName->text())) {
            KMessageBoxHelper::error(this, i18n("The branch already exists"));
            return;
        }
    accept();
}

#include "moc_switchbranchdialog.cpp"
