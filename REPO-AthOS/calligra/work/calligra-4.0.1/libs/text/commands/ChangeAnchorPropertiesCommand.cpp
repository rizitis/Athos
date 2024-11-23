/*
 *  SPDX-FileCopyrightText: 2012 C. Boemann <cbo@boemann.dk>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "ChangeAnchorPropertiesCommand.h"

#include "KoAnchorInlineObject.h"
#include "KoAnchorTextRange.h"

#include <KoInlineTextObjectManager.h>
#include <KoShapeContainer.h>
#include <KoTextDocument.h>
#include <KoTextEditor.h>
#include <KoTextRangeManager.h>
#include <KoTextShapeDataBase.h>

ChangeAnchorPropertiesCommand::ChangeAnchorPropertiesCommand(KoShapeAnchor *anchor,
                                                             const KoShapeAnchor &newAnchorData,
                                                             KoShapeContainer *newParent,
                                                             KUndo2Command *parent)
    : KUndo2Command(kundo2_noi18n("Change Anchor Properties"), parent)
    , m_anchor(anchor)
    , m_oldAnchor(nullptr)
    , m_newAnchor(nullptr)
    , m_oldParent(anchor->shape()->parent())
    , m_newParent(newParent)
    , m_oldLocation(anchor->textLocation())
    , m_newLocation(nullptr)
    , m_first(true)
    , m_undone(false)
{
    copyLayoutProperties(anchor, &m_oldAnchor);
    copyLayoutProperties(&newAnchorData, &m_newAnchor);
}

ChangeAnchorPropertiesCommand::~ChangeAnchorPropertiesCommand()
{
    if (m_undone) {
        delete m_newLocation;
    } else {
        delete m_oldLocation;
    }
}

void ChangeAnchorPropertiesCommand::copyLayoutProperties(const KoShapeAnchor *from, KoShapeAnchor *to)
{
    to->setAnchorType(from->anchorType());
    to->setOffset(from->offset());
    to->setVerticalPos(from->verticalPos());
    to->setVerticalRel(from->verticalRel());
    to->setHorizontalPos(from->horizontalPos());
    to->setHorizontalRel(from->horizontalRel());
}

void ChangeAnchorPropertiesCommand::redo()
{
    KoTextShapeDataBase *textData = nullptr;
    if (m_oldParent) {
        textData = qobject_cast<KoTextShapeDataBase *>(m_oldParent->userData());
    } else if (m_newParent) {
        textData = qobject_cast<KoTextShapeDataBase *>(m_newParent->userData());
    }

    KUndo2Command::redo();

    copyLayoutProperties(&m_newAnchor, m_anchor);

    m_anchor->shape()->update();
    if (m_first) {
        m_oldAbsPos = m_anchor->shape()->absolutePosition();
        m_anchor->shape()->setParent(m_newParent);
        // let's just set the old absolute position so it doesn't look like it's moving around
        m_anchor->shape()->setAbsolutePosition(m_oldAbsPos);
    } else {
        m_anchor->shape()->setParent(m_newParent);
        m_anchor->shape()->setAbsolutePosition(m_newAbsPos);
        m_anchor->shape()->update();
    }

    if (m_newAnchor.anchorType() != m_oldAnchor.anchorType()) {
        Q_ASSERT(textData);
        KoTextDocument doc(textData->document());
        KoInlineTextObjectManager *inlineManager = doc.inlineTextObjectManager();
        Q_ASSERT(inlineManager);
        KoTextRangeManager *rangeManager = doc.textRangeManager();
        Q_ASSERT(rangeManager);

        // First remove from the old location
        switch (m_oldAnchor.anchorType()) {
        case KoShapeAnchor::AnchorPage:
            // nothing we need to do to clean up old
            break;

        case KoShapeAnchor::AnchorAsCharacter:
            if (m_first) {
                // first time we need to remove the character manually
                QTextCursor cursor(textData->document());
                cursor.setPosition(m_oldLocation->position());
                cursor.deleteChar();
            }
            inlineManager->removeInlineObject(dynamic_cast<KoInlineObject *>(m_oldLocation));
            break;

        case KoShapeAnchor::AnchorParagraph:
        case KoShapeAnchor::AnchorToCharacter:
            rangeManager->remove(dynamic_cast<KoTextRange *>(m_oldLocation));

            // we need to mark dirty manually as it's a textrange
            textData->document()->markContentsDirty(m_oldLocation->position(), 0);
            break;
        }

        // And then set the new location
        switch (m_newAnchor.anchorType()) {
        case KoShapeAnchor::AnchorPage:
            m_anchor->setTextLocation(nullptr);
            break;

        case KoShapeAnchor::AnchorAsCharacter:
            if (m_first) {
                KoTextEditor *editor = doc.textEditor();
                QTextCursor cursor(textData->document());
                cursor.setPosition(editor->position());

                m_newLocation = new KoAnchorInlineObject(m_anchor);

                inlineManager->insertInlineObject(cursor, dynamic_cast<KoInlineObject *>(m_newLocation));
            } else {
                // only insert in manager as qt re-inserts the character
                inlineManager->addInlineObject(dynamic_cast<KoInlineObject *>(m_newLocation));
            }
            m_anchor->setTextLocation(m_newLocation);
            break;

        case KoShapeAnchor::AnchorParagraph:
        case KoShapeAnchor::AnchorToCharacter:
            if (m_first) {
                KoTextEditor *editor = doc.textEditor();
                QTextCursor cursor(textData->document());
                cursor.setPosition(editor->position());

                KoAnchorTextRange *anchorRange = new KoAnchorTextRange(m_anchor, cursor);
                anchorRange->setManager(rangeManager);
                rangeManager->insert(anchorRange);

                m_newLocation = anchorRange;
                rangeManager->insert(anchorRange);
            } else {
                rangeManager->insert(dynamic_cast<KoTextRange *>(m_newLocation));
            }
            m_anchor->setTextLocation(m_newLocation);

            // we need to mark dirty manually as it's a textrange
            textData->document()->markContentsDirty(m_newLocation->position(), 0);
            break;
        }
    } else if (m_newAnchor.anchorType() != KoShapeAnchor::AnchorPage) {
        if (textData) {
            Q_ASSERT(m_anchor->textLocation());
            textData->document()->markContentsDirty(m_anchor->textLocation()->position(), 0);
        }
    }

    m_first = false;
    m_undone = false;

    m_anchor->shape()->notifyChanged();
}

void ChangeAnchorPropertiesCommand::undo()
{
    KoTextShapeDataBase *textData = nullptr;
    if (m_oldParent) {
        textData = qobject_cast<KoTextShapeDataBase *>(m_oldParent->userData());
    } else if (m_newParent) {
        textData = qobject_cast<KoTextShapeDataBase *>(m_newParent->userData());
    }

    copyLayoutProperties(&m_oldAnchor, m_anchor);

    m_newAbsPos = m_anchor->shape()->absolutePosition();

    m_anchor->shape()->update();
    m_anchor->shape()->setParent(m_oldParent);
    m_anchor->shape()->setAbsolutePosition(m_oldAbsPos);
    m_anchor->shape()->update();

    if (m_newAnchor.anchorType() != m_oldAnchor.anchorType()) {
        Q_ASSERT(textData);
        KoTextDocument doc(textData->document());
        KoInlineTextObjectManager *inlineManager = doc.inlineTextObjectManager();
        Q_ASSERT(inlineManager);
        KoTextRangeManager *rangeManager = doc.textRangeManager();
        Q_ASSERT(rangeManager);

        // First unset the 'new' (current) location
        switch (m_newAnchor.anchorType()) {
        case KoShapeAnchor::AnchorPage:
            // nothing we need to do to clean up old
            break;

        case KoShapeAnchor::AnchorAsCharacter:
            // only remove in manager as qt removes the character
            inlineManager->removeInlineObject(dynamic_cast<KoInlineObject *>(m_newLocation));
            break;

        case KoShapeAnchor::AnchorParagraph:
        case KoShapeAnchor::AnchorToCharacter:
            rangeManager->remove(dynamic_cast<KoTextRange *>(m_newLocation));

            // we need to mark dirty manually as it's a textrange
            textData->document()->markContentsDirty(m_newLocation->position(), 0);
            break;
        }

        // Them re-insert the old (about to be restored) location
        switch (m_oldAnchor.anchorType()) {
        case KoShapeAnchor::AnchorPage:
            // nothing we need to do to clean up old
            m_anchor->setTextLocation(nullptr);
            break;

        case KoShapeAnchor::AnchorAsCharacter:
            // only insert in manager as qt re-inserts the character
            inlineManager->addInlineObject(dynamic_cast<KoInlineObject *>(m_oldLocation));
            m_anchor->setTextLocation(m_oldLocation);
            break;

        case KoShapeAnchor::AnchorParagraph:
        case KoShapeAnchor::AnchorToCharacter:
            rangeManager->insert(dynamic_cast<KoTextRange *>(m_oldLocation));

            // we need to mark dirty manually as it's a textrange
            textData->document()->markContentsDirty(m_oldLocation->position(), 0);
            m_anchor->setTextLocation(m_oldLocation);
            break;
        }
    } else if (m_newAnchor.anchorType() != KoShapeAnchor::AnchorPage) {
        if (textData) {
            Q_ASSERT(m_anchor->textLocation());
            textData->document()->markContentsDirty(m_anchor->textLocation()->position(), 0);
        }
    }

    KUndo2Command::undo();

    m_undone = true;

    m_anchor->shape()->notifyChanged();
}
