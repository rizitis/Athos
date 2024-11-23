/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006-2007 Thomas Zander <zander@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "FixedDateFormat.h"
#include "DateVariable.h"

#include <KoIcon.h>

#include <KLocalizedString>

#include <QAction>
#include <QLocale>
#include <QMenu>

static void createTimeAction(QMenu *parent, const QString &title, const QString &data)
{
    QAction *action = new QAction(title, parent);
    action->setData(data);
    parent->addAction(action);
}

FixedDateFormat::FixedDateFormat(DateVariable *variable)
    : m_variable(variable)
    , m_popup(nullptr)
{
    widget.setupUi(this);

    widget.normalPage->layout()->setContentsMargins({});
    widget.customPage->layout()->setContentsMargins({});

    QStringList listDateFormat;
    listDateFormat << i18n("Locale date format");
    listDateFormat << i18n("Short locale date format");
    listDateFormat << i18n("Locale date & time format");
    listDateFormat << i18n("Short locale date & time format");
    listDateFormat << "dd/MM/yy";
    listDateFormat << "dd/MM/yyyy";
    listDateFormat << "MMM dd,yy";
    listDateFormat << "MMM dd,yyyy";
    listDateFormat << "dd.MMM.yyyy";
    listDateFormat << "MMMM dd, yyyy";
    listDateFormat << "ddd, MMM dd,yy";
    listDateFormat << "dddd, MMM dd,yy";
    listDateFormat << "MM-dd";
    listDateFormat << "yyyy-MM-dd";
    listDateFormat << "dd/yy";
    listDateFormat << "MMMM";
    listDateFormat << "yyyy-MM-dd hh:mm";
    listDateFormat << "dd.MMM.yyyy hh:mm";
    listDateFormat << "MMM dd,yyyy h:mm AP";
    listDateFormat << "yyyy-MM-ddThh:mm:ss"; // ISO 8601
    widget.formatList->addItems(listDateFormat);
    widget.customString->setText(variable->definition());

    int index = listDateFormat.indexOf(variable->definition());
    if (index >= 0) {
        widget.widgetStack->setCurrentWidget(widget.normalPage);
        widget.formatList->item(index)->setSelected(true);
    } else {
        widget.widgetStack->setCurrentWidget(widget.customPage);
        widget.custom->setChecked(true);
    }

    widget.formatButton->setIcon(koIcon("list-add"));

    connect(widget.custom, &QCheckBox::stateChanged, this, &FixedDateFormat::customClicked);
    connect(widget.formatList, &QListWidget::itemPressed, this, &FixedDateFormat::listClicked);
    connect(widget.correction, &QSpinBox::valueChanged, this, &FixedDateFormat::offsetChanged);
    connect(widget.formatButton, &QAbstractButton::clicked, this, &FixedDateFormat::insertCustomButtonPressed);
    connect(widget.customString, &QLineEdit::textChanged, this, &FixedDateFormat::customTextChanged);
}

void FixedDateFormat::customClicked(int state)
{
    if (state == Qt::Unchecked)
        widget.widgetStack->setCurrentWidget(widget.normalPage);
    else
        widget.widgetStack->setCurrentWidget(widget.customPage);
}

void FixedDateFormat::listClicked(QListWidgetItem *item)
{
    // TODO parse out the first two values...
    QString format;
    switch (widget.formatList->row(item)) {
    case 0:
        format = QLocale().dateFormat(QLocale::LongFormat);
        break;
    case 1:
        format = QLocale().dateFormat(QLocale::ShortFormat);
        break;
    case 2:
        format = QLocale().dateTimeFormat(QLocale::LongFormat);
        break;
    case 3:
        format = QLocale().dateTimeFormat(QLocale::ShortFormat);
        break;
    default:
        format = item->text();
    }
    m_variable->setDefinition(format);
    widget.customString->setText(format);
}

void FixedDateFormat::offsetChanged(int offset)
{
    m_variable->setDaysOffset(offset);
}

void FixedDateFormat::insertCustomButtonPressed()
{
    if (m_popup == nullptr) {
        m_popup = new QMenu(this);
        QMenu *day = new QMenu(i18n("Day"), m_popup);
        QMenu *month = new QMenu(i18n("Month"), m_popup);
        QMenu *year = new QMenu(i18n("Year"), m_popup);
        QMenu *hour = new QMenu(i18n("Hour"), m_popup);
        QMenu *minute = new QMenu(i18n("Minute"), m_popup);
        QMenu *second = new QMenu(i18n("Second"), m_popup);
        m_popup->addMenu(day);
        m_popup->addMenu(month);
        m_popup->addMenu(year);
        m_popup->addMenu(hour);
        m_popup->addMenu(minute);
        m_popup->addMenu(second);

        createTimeAction(day, i18n("Flexible Digits (1-31)"), "d");
        createTimeAction(day, i18n("2 Digits (01-31)"), "dd");
        createTimeAction(day, i18n("Abbreviated Name"), "ddd");
        createTimeAction(day, i18n("Long Name"), "dddd");
        createTimeAction(month, i18n("Flexible Digits (1-12)"), "M");
        createTimeAction(month, i18n("2 Digits (01-12)"), "MM");
        createTimeAction(month, i18n("Abbreviated Name"), "MMM");
        createTimeAction(month, i18n("Long Name"), "MMMM");
        createTimeAction(month, i18n("Possessive Abbreviated Name"), "PPP");
        createTimeAction(month, i18n("Possessive Long Name"), "PPPP");
        createTimeAction(year, i18n("2 Digits (01-99)"), "yy");
        createTimeAction(year, i18n("4 Digits"), "yyyy");
        createTimeAction(hour, i18n("Flexible Digits (1-23)"), "h");
        createTimeAction(hour, i18n("2 Digits (01-23)"), "hh");
        createTimeAction(minute, i18n("Flexible Digits (1-59)"), "m");
        createTimeAction(minute, i18n("2 Digits (01-59)"), "mm");
        createTimeAction(second, i18n("Flexible Digits (1-59)"), "s");
        createTimeAction(second, i18n("2 Digits (01-59)"), "ss");
        createTimeAction(m_popup, i18n("am/pm"), "ap");
        createTimeAction(m_popup, i18n("AM/PM"), "AP");
    }
    QPoint position = widget.formatButton->mapToGlobal(QPoint(0, widget.formatButton->height()));
    QAction *action = m_popup->exec(position);
    if (action)
        widget.customString->insert(qvariant_cast<QString>(action->data()));
}

void FixedDateFormat::customTextChanged(const QString &text)
{
    m_variable->setDefinition(text);

    if (widget.custom->isChecked()) {
        // altering the custom text will deselect the list item so the user can easily switch
        // back by selecting one.
        QListWidgetItem *item = widget.formatList->currentItem();
        if (item) // deselect it.
            item->setSelected(false);
    }
}
