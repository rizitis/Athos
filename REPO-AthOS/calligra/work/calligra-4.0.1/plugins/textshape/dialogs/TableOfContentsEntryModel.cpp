/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2011 Gopalakrishna Bhat A <gopalakbhat@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "TableOfContentsEntryModel.h"
#include <KLocalizedString>
#include <KoParagraphStyle.h>
#include <KoStyleManager.h>
#include <KoTableOfContentsGeneratorInfo.h>

TableOfContentsEntryModel::TableOfContentsEntryModel(KoStyleManager *manager, KoTableOfContentsGeneratorInfo *info)
    : m_styleManager(manager)
    , m_tocInfo(info)
{
    Q_ASSERT(manager);
    Q_ASSERT(info);

    int titleStyleId = 0;
    if (m_styleManager->paragraphStyle(m_tocInfo->m_indexTitleTemplate.styleId)) {
        titleStyleId = m_tocInfo->m_indexTitleTemplate.styleId;
    } else {
        titleStyleId = m_styleManager->defaultParagraphStyle()->styleId();
    }

    m_tocEntries.append(qMakePair(i18n("Title"), titleStyleId));

    for (int i = 1; i <= m_tocInfo->m_outlineLevel; i++) {
        m_tocEntries.append(qMakePair(i18n("Level %1", QString("%1").arg(i)), m_styleManager->defaultTableOfContentsEntryStyle(i)->styleId()));
    }

    for (int j = 0; j < m_tocInfo->m_entryTemplate.count(); j++) {
        if (m_tocInfo->m_entryTemplate.at(j).outlineLevel <= 0 || m_tocInfo->m_entryTemplate.at(j).outlineLevel > m_tocInfo->m_outlineLevel) {
            continue; // ignore entries with outline level less than 0 and greater than the max outline level
        }
        if (m_styleManager->paragraphStyle(m_tocInfo->m_entryTemplate.at(j).styleId)) {
            m_tocEntries[m_tocInfo->m_entryTemplate.at(j).outlineLevel].second = m_tocInfo->m_entryTemplate.at(j).styleId;
        }
    }
}

int TableOfContentsEntryModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return m_tocEntries.count();
    }

    return 0;
}

int TableOfContentsEntryModel::columnCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return 2;
    }

    return 0;
}

QModelIndex TableOfContentsEntryModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || column < 0 || column > 1) {
        return QModelIndex();
    }

    if (!parent.isValid()) {
        if (row >= m_tocEntries.count()) {
            return QModelIndex();
        }
        return createIndex(row, column, new QPair<QString, int>(m_tocEntries[row].first, m_tocEntries[row].second));
    }
    return QModelIndex();
}

QVariant TableOfContentsEntryModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.column() == Levels) {
        switch (role) {
        case Qt::DisplayRole:
        case Qt::DecorationRole:
            return QVariant(static_cast<QPair<QString, int> *>(index.internalPointer())->first);
        default:
            break;
        }
    } else {
        switch (role) {
        case Qt::DisplayRole:
        case Qt::DecorationRole:
            return QVariant(m_styleManager->paragraphStyle(static_cast<QPair<QString, int> *>(index.internalPointer())->second)->name());
            // break
        case Qt::EditRole:
            return QVariant(static_cast<QPair<QString, int> *>(index.internalPointer())->second);
        default:
            break;
        }
    }
    return QVariant();
}

bool TableOfContentsEntryModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid()) {
        return false;
    }

    static_cast<QPair<int, int> *>(index.internalPointer())->second = value.toInt();
    QAbstractTableModel::setData(index, value, role);
    m_tocEntries[index.row()].second = value.toInt();

    // show data in preview
    saveData();
    Q_EMIT tocEntryDataChanged();

    return true;
}

Qt::ItemFlags TableOfContentsEntryModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return {};
    }
    if (index.column() == Levels) {
        return (Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    }
    if (index.column() == Styles) {
        return (Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
    }
    return {};
}

QVariant TableOfContentsEntryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section == Levels) {
            return i18n("Level");
        } else if (section == Styles) {
            return i18n("Style");
        } else {
            return QAbstractTableModel::headerData(section, orientation, role);
        }
    } else {
        return QAbstractTableModel::headerData(section, orientation, role);
    }
}

void TableOfContentsEntryModel::saveData()
{
    // index 0 of m_tocEntries is for title information
    m_tocInfo->m_indexTitleTemplate.styleName = m_styleManager->paragraphStyle(m_tocEntries.at(0).second)->name();
    m_tocInfo->m_indexTitleTemplate.styleId = m_tocEntries.at(0).second;

    for (int i = 1; i <= m_tocInfo->m_outlineLevel; i++) {
        m_tocInfo->m_entryTemplate[i - 1].styleName = m_styleManager->paragraphStyle(m_tocEntries.at(i).second)->name();
        m_tocInfo->m_entryTemplate[i - 1].styleId = m_tocEntries.at(i).second;
    }
}
