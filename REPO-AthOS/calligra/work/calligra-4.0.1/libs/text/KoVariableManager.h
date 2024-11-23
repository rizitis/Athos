/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2007 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2008, 2011 Sebastian Sauer <mail@dipe.org>
 * SPDX-FileCopyrightText: 2011 Robert Mathias Marmorstein <robert@narnia.homeunix.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef KOVARIABLEMANAGER_H
#define KOVARIABLEMANAGER_H

#include "kotext_export.h"

#include <KoXmlReaderForward.h>
#include <QObject>
#include <QString>

class KoXmlWriter;
class KoVariable;
class KoInlineTextObjectManager;
class KoVariableManagerPrivate;

/**
 * A document can maintain a list of name-value pairs, which we call variables.
 * These initially exist solely in the variableManager as such name/value pairs
 * and can be managed by setValue(), value() and remove().
 * When the user chooses to use one of these pairs in the text-document they can create a
 * new KoNamedVariable by calling KoVariableManager::createVariable()
 * use that and insert that into the text-document.
 * Changing the value will lead to directly change the value of all variables
 * inserted into the document.
 * @see KoInlineTextObjectManager::createInsertVariableActions()
 * @see KoInlineTextObjectManager::variableManager()
 */
class KOTEXT_EXPORT KoVariableManager : public QObject
{
    Q_OBJECT
public:
    /// constructor
    explicit KoVariableManager(KoInlineTextObjectManager *inlineObjectManager);
    ~KoVariableManager() override;

    /**
     * Set or create a variable to the new value.
     * @param name the name of the variable.
     * @param value the new value.
     * @param type The type of the value. This is only set for user-defined variables.
     * For non user defined variables the value is empty. If set then it should be one
     * of the following values;
     * \li string
     * \li boolean
     * \li currency
     * \li date
     * \li float
     * \li percentage
     * \li time
     * \li void
     * \li formula
     */
    void setValue(const QString &name, const QString &value, const QString &type = QString());

    /**
     * Remove a variable from the store.
     * Variables that were created and inserted into text will no longer get updated, but will keep
     * showing the proper text.
     * @see usageCount()
     */
    void remove(const QString &name);

    /**
     * Return the value a named variable currently has. Or an empty string if none.
     */
    QString value(const QString &name) const;

    /**
     * Return the type of a user defined variable. If the variable does not exist or
     * is not a user defined variable then an empty string is returned.
     */
    QString userType(const QString &name) const;

    /**
     * Create a new variable that can be inserted into the document using
     * KoInlineTextObjectManager::insertInlineObject()
     * This is a factory method that creates a visible variable object of an already existing
     * name/value pair previously inserted into the manager.
     * @param name the named variable.
     * @return the new variable, or 0 when the name was not previously set on this manager
     * @see setValue()
     */
    KoVariable *createVariable(const QString &name) const;

    /// return a list of all variable names.
    QList<QString> variables() const;
    /// return a list of all user defined variable names.
    QList<QString> userVariables() const;

    /**
     * Load user defined variable declarations from the ODF body-element.
     */
    void loadOdf(const KoXmlElement &bodyElement);

    /**
     * Save user defined variable declarations into the ODF writer.
     */
    void saveOdf(KoXmlWriter *bodyWriter);

Q_SIGNALS:
    void valueChanged();

private:
    KoVariableManagerPrivate *const d;
};

#endif
