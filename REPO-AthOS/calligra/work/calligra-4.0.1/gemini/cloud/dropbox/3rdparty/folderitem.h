/*

    SPDX-FileCopyrightText: 2011 Cuong Le <metacuong@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later

*/

#ifndef FOLDERITEM_H
#define FOLDERITEM_H

#include "listmodel.h"
#include <QObject>

/*
 * "revision":6226,
 * "thumb_exists":false,
 * "bytes":916480,
 * "modified":"Fri, 25 Feb 2011 08:25:31 +0000",
 * "path":"/2011-Craftly.ppt",
 * "is_dir":false,
 * "icon":"page_white_powerpoint",
 * "mime_type":"application/vnd.ms-powerpoint",
 * "size":"895KB"
 */

class FolderItem : public ListItem
{
    Q_OBJECT
    Q_PROPERTY(bool checked READ checked WRITE setChecked NOTIFY checkedChanged)
    Q_PROPERTY(qreal revision READ revision WRITE setRevision NOTIFY revisionChanged)
    Q_PROPERTY(bool thumb_exists READ thumb_exists WRITE setThumb_exists NOTIFY thumb_existsChanged)
    Q_PROPERTY(qreal bytes READ bytes WRITE setBytes NOTIFY bytesChanged)
    Q_PROPERTY(QString modified READ modified WRITE setModified NOTIFY modifiedChanged)
    Q_PROPERTY(QString path READ path WRITE setPath NOTIFY pathChanged)
    Q_PROPERTY(bool is_dir READ is_dir WRITE setIs_dir NOTIFY is_dirChanged)
    Q_PROPERTY(QString icon READ icon WRITE setIcon NOTIFY iconChanged)
    Q_PROPERTY(QString mime_type READ mime_type WRITE setMime_type NOTIFY mime_typeChanged)
    Q_PROPERTY(QString size READ size WRITE setSize NOTIFY sizeChanged)
    Q_PROPERTY(QString name READ name)
    Q_PROPERTY(QString section READ section)

public:
    enum Roles {
        RevisionRole = Qt::UserRole + 1,
        Thumb_existsRole,
        BytesRole,
        ModifiedRole,
        PathRole,
        Is_dirRole,
        IconRole,
        Mime_typeRole,
        SizeRole,
        CheckedRole,
        NameRole,
        SectionRole
    };

    explicit FolderItem(const qreal &revision,
                        const bool &thumb_exists,
                        const qreal &bytes,
                        const QString &modified,
                        const QString &path,
                        const bool &is_dir,
                        const QString &icon,
                        const QString &mime_type,
                        const QString &size,
                        QObject *parent = nullptr)
        : ListItem(parent)
        , m_revision(revision)
        , m_thumb_exists(thumb_exists)
        , m_bytes(bytes)
        , m_modified(modified)
        , m_path(path)
        , m_is_dir(is_dir)
        , m_icon(icon)
        , m_mime_type(mime_type)
        , m_size(size)
        , m_checked(false)
    {
    }

    FolderItem(QObject *parent = nullptr)
        : ListItem(parent)
    {
    }

    QVariant data(int role) const override;
    QHash<int, QByteArray> roleNames() const;

    inline QString id() const override
    {
        return m_path;
    }
    inline qreal revision() const
    {
        return m_revision;
    }
    inline bool thumb_exists() const
    {
        return m_thumb_exists;
    }
    inline qreal bytes() const
    {
        return m_bytes;
    }
    inline QString modified() const
    {
        return m_modified;
    }
    inline QString path() const
    {
        return m_path;
    }
    inline bool is_dir() const
    {
        return m_is_dir;
    }
    inline QString icon() const
    {
        return m_icon;
    }
    inline QString mime_type() const
    {
        return m_mime_type;
    }
    inline QString size() const
    {
        return m_size;
    }
    inline bool checked() const
    {
        return m_checked;
    }
    QString name() const;
    QString section() const;
    QString xsection();

    Q_INVOKABLE void setChecked(const bool &v);
    Q_INVOKABLE void setRevision(const qreal &v);
    Q_INVOKABLE void setThumb_exists(const bool &v);
    Q_INVOKABLE void setBytes(const qreal &v);
    Q_INVOKABLE void setModified(const QString &v);
    Q_INVOKABLE void setPath(const QString &v);
    Q_INVOKABLE void setIs_dir(const bool &v);
    Q_INVOKABLE void setIcon(const QString &v);
    Q_INVOKABLE void setMime_type(const QString &v);
    Q_INVOKABLE void setSize(const QString &v);

Q_SIGNALS:
    void checkedChanged();
    void revisionChanged();
    void thumb_existsChanged();
    void bytesChanged();
    void modifiedChanged();
    void pathChanged();
    void is_dirChanged();
    void iconChanged();
    void mime_typeChanged();
    void sizeChanged();

private:
    qreal m_revision;
    bool m_thumb_exists;
    qreal m_bytes;
    QString m_modified;
    QString m_path;
    bool m_is_dir;
    QString m_icon;
    QString m_mime_type;
    QString m_size;
    bool m_checked;
};

#endif // FOLDERITEM_H
