/*
SPDX-FileCopyrightText: 2021 Hamed Masafi <hamed.masfi@gmail.com>

SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "pulldialog.h"
#include "runnerdialog.h"

#include "commands/commandpull.h"
#include "gitmanager.h"
#include <QDialogButtonBox>
#include <QPushButton>

PullDialog::PullDialog(Git::Manager *git, QWidget *parent)
    : AppDialog(git, parent)
{
    setupUi(this);

    comboBoxRemote->addItems(git->remotes());
    comboBoxBranch->addItems(git->branchesNames(Git::Manager::BranchType::LocalBranch));

    comboBoxBranch->setCurrentText(git->currentBranch());
    connect(buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked, this, &PullDialog::slotAccepted);

    initComboBox<Git::CommandPull::Rebase>(comboBoxRebase);
    initComboBox<Git::CommandPull::FastForward>(comboBoxFastForward);
}

void PullDialog::slotAccepted()
{
    Git::CommandPull *cmd = new Git::CommandPull;

    cmd->setRemote(comboBoxRemote->currentText());
    cmd->setBranch(comboBoxBranch->currentText());
    cmd->setSquash(checkBoxSquash->isChecked());

    cmd->setNoCommit(checkBoxNoCommit->isChecked());
    cmd->setPrune(checkBoxPrune->isChecked());
    cmd->setTags(checkBoxTags->isChecked());

    cmd->setRebase(comboBoxCurrentValue<Git::CommandPull::Rebase>(comboBoxRebase));
    cmd->setFastForward(comboBoxCurrentValue<Git::CommandPull::FastForward>(comboBoxFastForward));

    RunnerDialog d(mGit, this);
    d.run(cmd);
    d.exec();

    accept();
}
#include "moc_pulldialog.cpp"
