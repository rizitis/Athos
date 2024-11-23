/*
SPDX-FileCopyrightText: 2021 Hamed Masafi <hamed.masfi@gmail.com>

SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

#include <QAbstractItemModel>

#include "libkommitwidgets_export.h"
#include <QIcon>

class LIBKOMMITWIDGETS_EXPORT TreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    struct Node {
        QString title;
        QList<Node *> childs;
        Node *parent;
        struct Feature *feature;
        int row{};
        QStringList data;
        QString key;
        QString prefix;
#ifdef TREEMODEL_NODE_DATA_TYPE
        TREEMODEL_NODE_DATA_TYPE metaData;
#endif
        Node()
            : parent(nullptr)
            , feature(nullptr)
        {
        }
        explicit Node(Node *parent)
            : parent(parent)
            , feature(nullptr)
        {
            row = parent->childs.count();
        }

        ~Node()
        {
            qDeleteAll(childs);
        }

        Node *createChild()
        {
            auto ch = new Node;
            ch->parent = this;
            ch->row = childs.count();
            childs.append(ch);
            return ch;
        }
        Node *find(const QString &title)
        {
            auto i = childs.begin();
            while (i != childs.end()) {
                if ((*i)->title == title)
                    return *i;
                ++i;
            }
            return nullptr;
        }

        void clear()
        {
            qDeleteAll(childs);
            childs.clear();
        }
    };

    explicit TreeModel(QObject *parent = nullptr);
    ~TreeModel() override;

    QModelIndex index(const Node *node, int col) const;
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Q_REQUIRED_RESULT QStringList rootData() const;
    Q_REQUIRED_RESULT QStringList data(const QModelIndex &index) const;
    Q_REQUIRED_RESULT QString fullPath(const QModelIndex &index) const;
    Q_REQUIRED_RESULT QString key(const QModelIndex &index) const;
    Q_REQUIRED_RESULT QString section(const QModelIndex &index) const;
    void sortItems();

    void addData(const QStringList &data, const QString &prefix = QString(), bool split = true);

    Q_REQUIRED_RESULT const QString &separator() const;
    void setSeparator(const QString &newSeparator);

    Q_REQUIRED_RESULT bool lastPartAsData() const;
    void setLastPartAsData(bool newLastPartAsData);

    Q_REQUIRED_RESULT const QIcon &defaultIcon() const;
    void setDefaultIcon(const QIcon &newDefaultIcon);

    void clear();

    Q_REQUIRED_RESULT bool showRoot() const;
    void setShowRoot(bool newDefaultRoot);

protected:
    Node *const mRootNode;

private:
    LIBKOMMITWIDGETS_NO_EXPORT Node *createPath(const QStringList &path);
    LIBKOMMITWIDGETS_NO_EXPORT Node *find(QStringList &path, Node *node = nullptr);
    LIBKOMMITWIDGETS_NO_EXPORT void getFullPath(QString &path, Node *node) const;
    LIBKOMMITWIDGETS_NO_EXPORT void sortNode(Node *node);

    QString mSeparator{QStringLiteral("/")};
    bool mLastPartAsData{false};
    QIcon mDefaultIcon;
    bool mShowRoot{false};
};
