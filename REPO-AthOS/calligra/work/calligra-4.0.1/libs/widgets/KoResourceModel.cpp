/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2008-2009 Jan Hambrecht <jaham@gmx.net>
 * SPDX-FileCopyrightText: 2013 Sascha Suelzer <s.suelzer@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KoResourceModel.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

#include <KoResourceServerAdapter.h>
#include <math.h>

KoResourceModel::KoResourceModel(QSharedPointer<KoAbstractResourceServerAdapter> resourceAdapter, QObject *parent)
    : KoResourceModelBase(parent)
    , m_resourceAdapter(resourceAdapter)
    , m_columnCount(4)
{
    Q_ASSERT(m_resourceAdapter);
    m_resourceAdapter->connectToResourceServer();

    connect(m_resourceAdapter.data(), &KoAbstractResourceServerAdapter::resourceAdded, this, &KoResourceModel::resourceAdded);
    connect(m_resourceAdapter.data(), &KoAbstractResourceServerAdapter::removingResource, this, &KoResourceModel::resourceRemoved);
    connect(m_resourceAdapter.data(), &KoAbstractResourceServerAdapter::resourceChanged, this, &KoResourceModel::resourceChanged);
    connect(m_resourceAdapter.data(), &KoAbstractResourceServerAdapter::tagsWereChanged, this, &KoResourceModel::tagBoxEntryWasModified);
    connect(m_resourceAdapter.data(), &KoAbstractResourceServerAdapter::tagCategoryWasAdded, this, &KoResourceModel::tagBoxEntryWasAdded);
    connect(m_resourceAdapter.data(), &KoAbstractResourceServerAdapter::tagCategoryWasRemoved, this, &KoResourceModel::tagBoxEntryWasRemoved);
}

KoResourceModel::~KoResourceModel()
{
    if (!m_currentTag.isEmpty()) {
        KConfigGroup group = KSharedConfig::openConfig()->group("SelectedTags");
        group.writeEntry(serverType(), m_currentTag);
    }
}

int KoResourceModel::rowCount(const QModelIndex & /*parent*/) const
{
    int resourceCount = m_resourceAdapter->resources().count();
    if (!resourceCount)
        return 0;

    return static_cast<int>(ceil(static_cast<qreal>(resourceCount) / m_columnCount));
}

int KoResourceModel::columnCount(const QModelIndex &) const
{
    return m_columnCount;
}

QVariant KoResourceModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    switch (role) {
    case Qt::DisplayRole: {
        KoResource *resource = static_cast<KoResource *>(index.internalPointer());
        if (!resource)
            return QVariant();
        QString resName = i18n(resource->name().toUtf8().data());

        if (m_resourceAdapter->assignedTagsList(resource).count()) {
            QString taglist = m_resourceAdapter->assignedTagsList(resource).join("] , [");
            QString tagListToolTip = QString(" - %1: [%2]").arg(i18n("Tags"), taglist);
            return QVariant(resName + tagListToolTip);
        }
        return QVariant(resName);
    }
    case Qt::DecorationRole: {
        KoResource *resource = static_cast<KoResource *>(index.internalPointer());
        if (!resource)
            return QVariant();

        return QVariant(resource->image());
    }
    case KoResourceModel::LargeThumbnailRole: {
        KoResource *resource = static_cast<KoResource *>(index.internalPointer());
        if (!resource)
            return QVariant();

        QSize imageSize = resource->image().size();
        QSize thumbSize(100, 100);
        if (imageSize.height() > thumbSize.height() || imageSize.width() > thumbSize.width()) {
            qreal scaleW = static_cast<qreal>(thumbSize.width()) / static_cast<qreal>(imageSize.width());
            qreal scaleH = static_cast<qreal>(thumbSize.height()) / static_cast<qreal>(imageSize.height());

            qreal scale = qMin(scaleW, scaleH);

            int thumbW = static_cast<int>(imageSize.width() * scale);
            int thumbH = static_cast<int>(imageSize.height() * scale);

            return QVariant(resource->image().scaled(thumbW, thumbH, Qt::IgnoreAspectRatio));
        } else
            return QVariant(resource->image());
    }

    default:
        return QVariant();
    }
}

QModelIndex KoResourceModel::index(int row, int column, const QModelIndex &) const
{
    int index = row * m_columnCount + column;
    const QList<KoResource *> resources = m_resourceAdapter->resources();
    if (index >= resources.count() || index < 0)
        return QModelIndex();

    return createIndex(row, column, resources[index]);
}

void KoResourceModel::doSafeLayoutReset(KoResource *activateAfterReformat)
{
    Q_EMIT beforeResourcesLayoutReset(activateAfterReformat);
    beginResetModel();
    endResetModel();
    Q_EMIT afterResourcesLayoutReset();
}

void KoResourceModel::setColumnCount(int columnCount)
{
    if (columnCount != m_columnCount) {
        Q_EMIT beforeResourcesLayoutReset(nullptr);
        beginResetModel();
        m_columnCount = columnCount;
        endResetModel();
        Q_EMIT afterResourcesLayoutReset();
    }
}

void KoResourceModel::resourceAdded(KoResource *resource)
{
    int newIndex = m_resourceAdapter->resources().indexOf(resource);
    if (newIndex >= 0) {
        doSafeLayoutReset(nullptr);
    }
}

void KoResourceModel::resourceRemoved(KoResource *resource)
{
    Q_UNUSED(resource);

    KoResource *first = !m_resourceAdapter->resources().isEmpty() ? m_resourceAdapter->resources().first() : 0;
    doSafeLayoutReset(first);
}

void KoResourceModel::resourceChanged(KoResource *resource)
{
    int resourceIndex = m_resourceAdapter->resources().indexOf(resource);
    int row = resourceIndex / columnCount();
    int column = resourceIndex % columnCount();

    QModelIndex modelIndex = index(row, column);
    if (!modelIndex.isValid()) {
        return;
    }

    Q_EMIT dataChanged(modelIndex, modelIndex);
}

void KoResourceModel::tagBoxEntryWasModified()
{
    m_resourceAdapter->updateServer();
    Q_EMIT tagBoxEntryModified();
}

void KoResourceModel::tagBoxEntryWasAdded(const QString &tag)
{
    Q_EMIT tagBoxEntryAdded(tag);
}

void KoResourceModel::tagBoxEntryWasRemoved(const QString &tag)
{
    Q_EMIT tagBoxEntryRemoved(tag);
}

QModelIndex KoResourceModel::indexFromResource(KoResource *resource) const
{
    int resourceIndex = m_resourceAdapter->resources().indexOf(resource);
    if (columnCount() > 0) {
        int row = resourceIndex / columnCount();
        int column = resourceIndex % columnCount();
        return index(row, column);
    }
    return QModelIndex();
}

QString KoResourceModel::extensions() const
{
    return m_resourceAdapter->extensions();
}

void KoResourceModel::importResourceFile(const QString &filename)
{
    m_resourceAdapter->importResourceFile(filename);
}

void KoResourceModel::importResourceFile(const QString &filename, bool fileCreation)
{
    m_resourceAdapter->importResourceFile(filename, fileCreation);
}

bool KoResourceModel::removeResource(KoResource *resource)
{
    return m_resourceAdapter->removeResource(resource);
}

void KoResourceModel::removeResourceFile(const QString &filename)
{
    m_resourceAdapter->removeResourceFile(filename);
}

QStringList KoResourceModel::assignedTagsList(KoResource *resource) const
{
    return m_resourceAdapter->assignedTagsList(resource);
}

void KoResourceModel::addTag(KoResource *resource, const QString &tag)
{
    m_resourceAdapter->addTag(resource, tag);
    Q_EMIT tagBoxEntryAdded(tag);
}

void KoResourceModel::deleteTag(KoResource *resource, const QString &tag)
{
    m_resourceAdapter->deleteTag(resource, tag);
}

QStringList KoResourceModel::tagNamesList() const
{
    return m_resourceAdapter->tagNamesList();
}

QStringList KoResourceModel::searchTag(const QString &lineEditText)
{
    return m_resourceAdapter->searchTag(lineEditText);
}

void KoResourceModel::searchTextChanged(const QString &searchString)
{
    m_resourceAdapter->searchTextChanged(searchString);
}

void KoResourceModel::enableResourceFiltering(bool enable)
{
    m_resourceAdapter->enableResourceFiltering(enable);
}

void KoResourceModel::setCurrentTag(const QString &currentTag)
{
    m_currentTag = currentTag;
    m_resourceAdapter->setCurrentTag(currentTag);
}

void KoResourceModel::updateServer()
{
    m_resourceAdapter->updateServer();
}

int KoResourceModel::resourcesCount() const
{
    return m_resourceAdapter->resources().count();
}

QList<KoResource *> KoResourceModel::currentlyVisibleResources() const
{
    return m_resourceAdapter->resources();
}

void KoResourceModel::tagCategoryMembersChanged()
{
    m_resourceAdapter->tagCategoryMembersChanged();
}

void KoResourceModel::tagCategoryAdded(const QString &tag)
{
    m_resourceAdapter->tagCategoryAdded(tag);
}

void KoResourceModel::tagCategoryRemoved(const QString &tag)
{
    m_resourceAdapter->tagCategoryRemoved(tag);
}

QString KoResourceModel::serverType() const
{
    return m_resourceAdapter->serverType();
}

QList<KoResource *> KoResourceModel::serverResources() const
{
    return m_resourceAdapter->serverResources();
}
