/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2009 Nokia Corporation and /or its subsidiary(-ies).

   Contact: Suresh Chande suresh.chande@nokia.com

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOFONTFACE_H
#define KOFONTFACE_H

#include "koodf_export.h"
#include <QSharedData>
#include <QString>

class KoXmlWriter;
class KoFontFacePrivate;

/**
 * @brief Represents font style.
 * Font style is defined by the style:font-face element.
 * @todo add more parameters.
 * @todo add class KoFontFaceDeclarations instead of adding methods to KoGenStyle?
 */
class KOODF_EXPORT KoFontFace
{
public:
    /**
     * Constructor. Creates font face definition with empty parameters.
     *
     * @param name the font name.
     *
     * The other are empty. If you don't pass the name, the font face will be considered null.
     * @see isEmpty()
     */
    explicit KoFontFace(const QString &name = QString());

    /**
     * Copy constructor.
     */
    KoFontFace(const KoFontFace &other);

    /**
     * Destructor.
     */
    ~KoFontFace();

    /**
     * @return true if the font face object is null, i.e. has no name assigned.
     */
    bool isNull() const;

    KoFontFace &operator=(const KoFontFace &other);

    bool operator==(const KoFontFace &other) const;

    enum Pitch { FixedPitch, VariablePitch };
    //! @todo add enum FamilyGeneric?

    QString name() const;
    void setName(const QString &name);
    QString family() const;
    void setFamily(const QString &family);
    QString familyGeneric() const;
    void setFamilyGeneric(const QString &familyGeneric);
    QString style() const;
    void setStyle(const QString &style);
    KoFontFace::Pitch pitch() const;
    void setPitch(KoFontFace::Pitch pitch);

    /** Saves font face definition into @a xmlWriter as a style:font-face element.
     */
    void saveOdf(KoXmlWriter *xmlWriter) const;

private:
    QSharedDataPointer<KoFontFacePrivate> d;
};

#endif /* KOFONTFACE_H */
