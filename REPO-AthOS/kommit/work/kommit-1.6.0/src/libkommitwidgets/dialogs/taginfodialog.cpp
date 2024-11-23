/*
SPDX-FileCopyrightText: 2021 Hamed Masafi <hamed.masfi@gmail.com>

SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "taginfodialog.h"
#include <QPushButton>

TagInfoDialog::TagInfoDialog(QWidget *parent)
    : AppDialog(parent)
{
    setupUi(this);
    setWindowTitle(i18nc("@title:window", "New tag"));

    connect(lineEditTagName, &QLineEdit::textChanged, this, &TagInfoDialog::slotLineEditTagNameTextChanged);
    connect(lineEditTagName, &QLineEdit::textChanged, this, &TagInfoDialog::updateOkButton);
    connect(lineEditMessage, &QLineEdit::textChanged, this, &TagInfoDialog::updateOkButton);
    mOkButton = buttonBox->button(QDialogButtonBox::Ok);
    mOkButton->setEnabled(false);
}

void TagInfoDialog::updateOkButton()
{
    mOkButton->setEnabled(!tagName().trimmed().isEmpty() && !message().trimmed().isEmpty());
}

QString TagInfoDialog::tagName() const
{
    return lineEditTagName->text();
}

void TagInfoDialog::setTagName(const QString &newTagName)
{
    lineEditTagName->setText(newTagName);
}

QString TagInfoDialog::message() const
{
    return lineEditMessage->text();
}

void TagInfoDialog::setMessage(const QString &newMessage)
{
    lineEditMessage->setText(newMessage);
}

void TagInfoDialog::slotLineEditTagNameTextChanged(const QString &s)
{
    lineEditMessage->setText(s);
}

#include "moc_taginfodialog.cpp"
