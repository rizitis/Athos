/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2007-2008 Fredy Yanardi <fyanardi@gmail.com>
 * SPDX-FileCopyrightText: 2011 Boudewijn Rempt <boud@kogmbh.com>
 * SPDX-FileCopyrightText: 2012 C. Boemann <cbo@boemann.dk>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KoBookmark.h"

#include <KoShapeSavingContext.h>
#include <KoTextInlineRdf.h>
#include <KoTextRangeManager.h>
#include <KoXmlNS.h>
#include <KoXmlReader.h>
#include <KoXmlWriter.h>

#include "TextDebug.h"
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>

// Include Q_UNSUSED classes, for building on Windows
#include <KoShapeLoadingContext.h>

class Q_DECL_HIDDEN KoBookmark::Private
{
public:
    Private(const QTextDocument *doc)
        : document(doc)
    {
    }
    const QTextDocument *document;
    QString name;
};

KoBookmark::KoBookmark(const QTextCursor &cursor)
    : KoTextRange(cursor)
    , d(new Private(cursor.block().document()))
{
}

KoBookmark::KoBookmark(QTextDocument *document, int position)
    : KoTextRange(document, position)
    , d(new Private(document))
{
}

KoBookmark::~KoBookmark()
{
    delete d;
}

void KoBookmark::setName(const QString &name)
{
    d->name = name;
}

QString KoBookmark::name() const
{
    return d->name;
}

bool KoBookmark::loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    Q_UNUSED(context);

    QString bookmarkName = element.attribute("name");
    const QString localName(element.localName());

    if (manager()) {
        // For cut and paste, make sure that the name is unique.
        d->name = createUniqueBookmarkName(manager()->bookmarkManager(), bookmarkName, false);

        if (localName == "bookmark" || localName == "bookmark-start") {
            setPositionOnlyMode(localName == "bookmark");

            // Add inline Rdf to the bookmark.
            if (element.hasAttributeNS(KoXmlNS::xhtml, "property") || element.hasAttribute("id")) {
                KoTextInlineRdf *inlineRdf = new KoTextInlineRdf(const_cast<QTextDocument *>(d->document), this);
                if (inlineRdf->loadOdf(element)) {
                    setInlineRdf(inlineRdf);
                } else {
                    delete inlineRdf;
                    inlineRdf = nullptr;
                }
            }
        } else {
            // NOTE: "bookmark-end" is handled in KoTextLoader
            // if we ever come here then something pretty weird is going on...
            return false;
        }
        return true;
    }
    return false;
}

void KoBookmark::saveOdf(KoShapeSavingContext &context, int position, TagType tagType) const
{
    KoXmlWriter *writer = &context.xmlWriter();

    if (!hasRange()) {
        if (tagType == StartTag) {
            writer->startElement("text:bookmark", false);
            writer->addAttribute("text:name", d->name.toUtf8());
            if (inlineRdf()) {
                inlineRdf()->saveOdf(context, writer);
            }
            writer->endElement();
        }
    } else if ((tagType == StartTag) && (position == rangeStart())) {
        writer->startElement("text:bookmark-start", false);
        writer->addAttribute("text:name", d->name.toUtf8());
        if (inlineRdf()) {
            inlineRdf()->saveOdf(context, writer);
        }
        writer->endElement();
    } else if ((tagType == EndTag) && (position == rangeEnd())) {
        writer->startElement("text:bookmark-end", false);
        writer->addAttribute("text:name", d->name.toUtf8());
        writer->endElement();
    }
    // else nothing
}

QString KoBookmark::createUniqueBookmarkName(const KoBookmarkManager *bmm, const QString &bookmarkName, bool isEndMarker)
{
    QString ret = bookmarkName;
    int uniqID = 0;

    while (true) {
        if (bmm->bookmark(ret)) {
            ret = QString("%1_%2").arg(bookmarkName).arg(++uniqID);
        } else {
            if (isEndMarker) {
                --uniqID;
                if (!uniqID)
                    ret = bookmarkName;
                else
                    ret = QString("%1_%2").arg(bookmarkName).arg(uniqID);
            }
            break;
        }
    }
    return ret;
}
