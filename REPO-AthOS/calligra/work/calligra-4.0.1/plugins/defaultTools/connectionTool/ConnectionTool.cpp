/* This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2009 Thorsten Zachmann <zachmann@kde.org>
 * SPDX-FileCopyrightText: 2009 Jean-Nicolas Artaud <jeannicolasartaud@gmail.com>
 * SPDX-FileCopyrightText: 2011 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "ConnectionTool.h"

#include "../../textshape/TextShape.h"
#include "AddConnectionPointCommand.h"
#include "ChangeConnectionPointCommand.h"
#include "ConnectionPointWidget.h"
#include "MoveConnectionPointStrategy.h"
#include "RemoveConnectionPointCommand.h"

#include <KoCanvasBase.h>
#include <KoConnectionShapeConfigWidget.h>
#include <KoDocumentResourceManager.h>
#include <KoInteractionStrategy.h>
#include <KoPathConnectionPointStrategy.h>
#include <KoPathSegment.h>
#include <KoPointerEvent.h>
#include <KoSelection.h>
#include <KoShape.h>
#include <KoShapeConfigWidgetBase.h>
#include <KoShapeController.h>
#include <KoShapeFactoryBase.h>
#include <KoShapeGroup.h>
#include <KoShapeLayer.h>
#include <KoShapeManager.h>
#include <KoShapeRegistry.h>
#include <KoStrokeConfigWidget.h>

#include <KoIcon.h>
#include <kundo2command.h>

#include <KLocalizedString>

#include <QAction>
#include <QDebug>
#include <QKeyEvent>
#include <QPainter>
#include <QPointF>
#include <QStandardPaths>

#include <algorithm>

ConnectionTool::ConnectionTool(KoCanvasBase *canvas)
    : KoToolBase(canvas)
    , m_editMode(Idle)
    , m_connectionType(KoConnectionShape::Standard)
    , m_currentShape(nullptr)
    , m_activeHandle(-1)
    , m_currentStrategy(nullptr)
    , m_oldSnapStrategies(KoSnapGuide::Strategies())
    , m_resetPaint(true)
{
    QPixmap connectPixmap;
    connectPixmap.load(QStandardPaths::locate(QStandardPaths::GenericDataLocation, "calligra/cursors/cursor_connect.png"));
    m_connectCursor = QCursor(connectPixmap, 4, 1);

    m_editConnectionPoint = new QAction(i18n("Edit connection points"), this);
    m_editConnectionPoint->setCheckable(true);
    addAction("toggle-edit-mode", m_editConnectionPoint);

    m_alignPercent = new QAction(QString("%"), this);
    m_alignPercent->setCheckable(true);
    addAction("align-relative", m_alignPercent);
    m_alignLeft = new QAction(koIcon("align-horizontal-left"), i18n("Align to left edge"), this);
    m_alignLeft->setCheckable(true);
    addAction("align-left", m_alignLeft);
    m_alignCenterH = new QAction(koIcon("align-horizontal-center"), i18n("Align to horizontal center"), this);
    m_alignCenterH->setCheckable(true);
    addAction("align-centerh", m_alignCenterH);
    m_alignRight = new QAction(koIcon("align-horizontal-right"), i18n("Align to right edge"), this);
    m_alignRight->setCheckable(true);
    addAction("align-right", m_alignRight);
    m_alignTop = new QAction(koIcon("align-vertical-top"), i18n("Align to top edge"), this);
    m_alignTop->setCheckable(true);
    addAction("align-top", m_alignTop);
    m_alignCenterV = new QAction(koIcon("align-vertical-center"), i18n("Align to vertical center"), this);
    m_alignCenterV->setCheckable(true);
    addAction("align-centerv", m_alignCenterV);
    m_alignBottom = new QAction(koIcon("align-vertical-bottom"), i18n("Align to bottom edge"), this);
    m_alignBottom->setCheckable(true);
    addAction("align-bottom", m_alignBottom);

    m_escapeAll = new QAction(koIcon("escape-direction-all"), i18n("Escape in all directions"), this);
    m_escapeAll->setCheckable(true);
    addAction("escape-all", m_escapeAll);
    m_escapeHorizontal = new QAction(koIcon("escape-direction-horizontal"), i18n("Escape in horizontal directions"), this);
    m_escapeHorizontal->setCheckable(true);
    addAction("escape-horizontal", m_escapeHorizontal);
    m_escapeVertical = new QAction(koIcon("escape-direction-vertical"), i18n("Escape in vertical directions"), this);
    m_escapeVertical->setCheckable(true);
    addAction("escape-vertical", m_escapeVertical);
    m_escapeLeft = new QAction(koIcon("escape-direction-left"), i18n("Escape in left direction"), this);
    m_escapeLeft->setCheckable(true);
    addAction("escape-left", m_escapeLeft);
    m_escapeRight = new QAction(koIcon("escape-direction-right"), i18n("Escape in right direction"), this);
    m_escapeRight->setCheckable(true);
    addAction("escape-right", m_escapeRight);
    m_escapeUp = new QAction(koIcon("escape-direction-up"), i18n("Escape in up direction"), this);
    m_escapeUp->setCheckable(true);
    addAction("escape-up", m_escapeUp);
    m_escapeDown = new QAction(koIcon("escape-direction-down"), i18n("Escape in down direction"), this);
    m_escapeDown->setCheckable(true);
    addAction("escape-down", m_escapeDown);

    m_alignHorizontal = new QActionGroup(this);
    m_alignHorizontal->setExclusive(true);
    m_alignHorizontal->addAction(m_alignLeft);
    m_alignHorizontal->addAction(m_alignCenterH);
    m_alignHorizontal->addAction(m_alignRight);
    connect(m_alignHorizontal, &QActionGroup::triggered, this, &ConnectionTool::horizontalAlignChanged);

    m_alignVertical = new QActionGroup(this);
    m_alignVertical->setExclusive(true);
    m_alignVertical->addAction(m_alignTop);
    m_alignVertical->addAction(m_alignCenterV);
    m_alignVertical->addAction(m_alignBottom);
    connect(m_alignVertical, &QActionGroup::triggered, this, &ConnectionTool::verticalAlignChanged);

    m_alignRelative = new QActionGroup(this);
    m_alignRelative->setExclusive(true);
    m_alignRelative->addAction(m_alignPercent);
    connect(m_alignRelative, &QActionGroup::triggered, this, &ConnectionTool::relativeAlignChanged);

    m_escapeDirections = new QActionGroup(this);
    m_escapeDirections->setExclusive(true);
    m_escapeDirections->addAction(m_escapeAll);
    m_escapeDirections->addAction(m_escapeHorizontal);
    m_escapeDirections->addAction(m_escapeVertical);
    m_escapeDirections->addAction(m_escapeLeft);
    m_escapeDirections->addAction(m_escapeRight);
    m_escapeDirections->addAction(m_escapeUp);
    m_escapeDirections->addAction(m_escapeDown);
    connect(m_escapeDirections, &QActionGroup::triggered, this, &ConnectionTool::escapeDirectionChanged);

    connect(this, &ConnectionTool::connectionPointEnabled, m_alignHorizontal, &QActionGroup::setEnabled);
    connect(this, &ConnectionTool::connectionPointEnabled, m_alignVertical, &QActionGroup::setEnabled);
    connect(this, &ConnectionTool::connectionPointEnabled, m_alignRelative, &QActionGroup::setEnabled);
    connect(this, &ConnectionTool::connectionPointEnabled, m_escapeDirections, &QActionGroup::setEnabled);

    connect(canvas->shapeManager(), &KoShapeManager::shapeRemoved, this, &ConnectionTool::slotShapeRemoved);

    resetEditMode();
}

ConnectionTool::~ConnectionTool() = default;

void ConnectionTool::paint(QPainter &painter, const KoViewConverter &converter)
{
    // get the correctly sized rect for painting handles
    QRectF handleRect = handlePaintRect(QPointF());

    painter.setRenderHint(QPainter::Antialiasing, true);

    if (m_currentStrategy) {
        painter.save();
        m_currentStrategy->paint(painter, converter);
        painter.restore();
    }

    QList<KoShape *> shapes = canvas()->shapeManager()->shapes();
    for (QList<KoShape *>::const_iterator end = shapes.constBegin(); end != shapes.constEnd(); ++end) {
        KoShape *shape = *end;
        if (!dynamic_cast<KoConnectionShape *>(shape)) {
            // only paint connection points of textShapes not inside a tos container and other shapes
            if (shape->shapeId() == TextShape_SHAPEID && dynamic_cast<KoTosContainer *>(shape->parent()))
                continue;

            painter.save();
            painter.setPen(QPen(Qt::black, 0));
            QTransform transform = shape->absoluteTransformation(nullptr);
            KoShape::applyConversion(painter, converter);
            // Draw all the connection points of the shape
            KoConnectionPoints connectionPoints = shape->connectionPoints();
            KoConnectionPoints::const_iterator cp = connectionPoints.constBegin();
            KoConnectionPoints::const_iterator lastCp = connectionPoints.constEnd();
            for (; cp != lastCp; ++cp) {
                if (shape == findNonConnectionShapeAtPosition(transform.map(cp.value().position))) {
                    handleRect.moveCenter(transform.map(cp.value().position));
                    painter.setBrush(cp.key() == m_activeHandle && shape == m_currentShape ? Qt::red : Qt::white);
                    painter.drawRect(handleRect);
                }
            }
            painter.restore();
        }
    }
    // paint connection points or connection handles depending
    // on the shape the mouse is currently
    if (m_currentShape && m_editMode == EditConnection) {
        KoConnectionShape *connectionShape = dynamic_cast<KoConnectionShape *>(m_currentShape);
        if (connectionShape) {
            int radius = handleRadius() + 1;
            int handleCount = connectionShape->handleCount();
            for (int i = 0; i < handleCount; ++i) {
                painter.save();
                painter.setPen(QPen(Qt::blue, 0));
                painter.setBrush(i == m_activeHandle ? Qt::red : Qt::white);
                painter.setTransform(connectionShape->absoluteTransformation(&converter) * painter.transform());
                connectionShape->paintHandle(painter, converter, i, radius);
                painter.restore();
            }
        }
    }
}

void ConnectionTool::repaintDecorations()
{
    const qreal radius = handleRadius();
    QRectF repaintRect;

    if (m_currentShape) {
        repaintRect = m_currentShape->boundingRect();
        canvas()->updateCanvas(repaintRect.adjusted(-radius, -radius, radius, radius));
        KoConnectionShape *connectionShape = dynamic_cast<KoConnectionShape *>(m_currentShape);
        if (!m_resetPaint && m_currentShape->isVisible(true) && !connectionShape) {
            // only paint connection points of textShapes not inside a tos container and other shapes
            if (!(m_currentShape->shapeId() == TextShape_SHAPEID && dynamic_cast<KoTosContainer *>(m_currentShape->parent()))) {
                KoConnectionPoints connectionPoints = m_currentShape->connectionPoints();
                KoConnectionPoints::const_iterator cp = connectionPoints.constBegin();
                KoConnectionPoints::const_iterator lastCp = connectionPoints.constEnd();
                for (; cp != lastCp; ++cp) {
                    repaintRect = handleGrabRect(m_currentShape->shapeToDocument(cp.value().position));
                    canvas()->updateCanvas(repaintRect.adjusted(-radius, -radius, radius, radius));
                }
            }
        }
        if (m_editMode == EditConnection) {
            if (connectionShape) {
                QPointF handlePos = connectionShape->handlePosition(m_activeHandle);
                handlePos = connectionShape->shapeToDocument(handlePos);
                repaintRect = handlePaintRect(handlePos);
                canvas()->updateCanvas(repaintRect.adjusted(-radius, -radius, radius, radius));
            }
        }
    }
    if (m_resetPaint) {
        QList<KoShape *> shapes = canvas()->shapeManager()->shapes();
        for (QList<KoShape *>::const_iterator end = shapes.constBegin(); end != shapes.constEnd(); ++end) {
            KoShape *shape = *end;
            if (!dynamic_cast<KoConnectionShape *>(shape)) {
                // only paint connection points of textShapes not inside a tos container and other shapes
                if (shape->shapeId() == TextShape_SHAPEID && dynamic_cast<KoTosContainer *>(shape->parent()))
                    continue;

                KoConnectionPoints connectionPoints = shape->connectionPoints();
                KoConnectionPoints::const_iterator cp = connectionPoints.constBegin();
                KoConnectionPoints::const_iterator lastCp = connectionPoints.constEnd();
                for (; cp != lastCp; ++cp) {
                    repaintRect = handleGrabRect(shape->shapeToDocument(cp.value().position));
                    canvas()->updateCanvas(repaintRect.adjusted(-radius, -radius, radius, radius));
                }
            }
        }
    }
    m_resetPaint = false;
}

void ConnectionTool::mousePressEvent(KoPointerEvent *event)
{
    if (!m_currentShape) {
        return;
    }

    KoShape *hitShape = findShapeAtPosition(event->point);
    int hitHandle = handleAtPoint(m_currentShape, event->point);

    if (m_editMode == EditConnection && hitHandle >= 0) {
        // create connection handle change strategy
        m_currentStrategy = new KoPathConnectionPointStrategy(this, dynamic_cast<KoConnectionShape *>(m_currentShape), hitHandle);
    } else if (m_editMode == EditConnectionPoint) {
        if (hitHandle >= KoConnectionPoint::FirstCustomConnectionPoint) {
            // start moving custom connection point
            m_currentStrategy = new MoveConnectionPointStrategy(m_currentShape, hitHandle, this);
        }
    } else if (m_editMode == CreateConnection) {
        // create new connection shape, connect it to the active connection point
        // and start editing the new connection
        // create the new connection shape
        KoShapeFactoryBase *factory = KoShapeRegistry::instance()->value("KoConnectionShape");
        KoShape *shape = nullptr;
        if (factory) {
            shape = factory->createDefaultShape(canvas()->shapeController()->resourceManager());
        }
        KoConnectionShape *connectionShape = dynamic_cast<KoConnectionShape *>(shape);
        if (!connectionShape) {
            delete shape;
            resetEditMode();
            return;
        }
        // set connection type
        connectionShape->setType(m_connectionType);
        // get the position of the connection point we start our connection from
        QPointF cp = m_currentShape->shapeToDocument(m_currentShape->connectionPoint(m_activeHandle).position);
        // move both handles to that point
        connectionShape->moveHandle(0, cp);
        connectionShape->moveHandle(1, cp);
        // connect the first handle of the connection shape to our connection point
        if (!connectionShape->connectFirst(m_currentShape, m_activeHandle)) {
            delete shape;
            resetEditMode();
            return;
        }
        // add connector label
        connectionShape->createTextShape(canvas()->shapeController()->resourceManager());
        connectionShape->setPlainText("");
        // create the connection edit strategy from the path tool
        m_currentStrategy = new KoPathConnectionPointStrategy(this, connectionShape, 1);
        if (!m_currentStrategy) {
            delete shape;
            resetEditMode();
            return;
        }
        // update our handle data
        setEditMode(m_editMode, shape, 1);
        // add connection shape to the shape manager so it gets painted
        canvas()->shapeManager()->addShape(connectionShape);
    } else {
        // pressing on a shape in idle mode switches to corresponding edit mode
        if (hitShape) {
            if (dynamic_cast<KoConnectionShape *>(hitShape)) {
                int hitHandle = handleAtPoint(hitShape, event->point);
                setEditMode(EditConnection, hitShape, hitHandle);
                if (hitHandle >= 0) {
                    // start editing connection shape
                    KoConnectionShape *shape = dynamic_cast<KoConnectionShape *>(m_currentShape);
                    if (shape) {
                        m_currentStrategy = new KoPathConnectionPointStrategy(this, shape, m_activeHandle);
                    }
                }
            }
        } else {
            resetEditMode();
        }
    }
}

void ConnectionTool::mouseMoveEvent(KoPointerEvent *event)
{
    if (m_currentStrategy) {
        repaintDecorations();
        if (m_editMode != EditConnection && m_editMode != CreateConnection) {
            QPointF snappedPos = canvas()->snapGuide()->snap(event->point, event->modifiers());
            m_currentStrategy->handleMouseMove(snappedPos, event->modifiers());
        } else {
            m_currentStrategy->handleMouseMove(event->point, event->modifiers());
        }
        repaintDecorations();
    } else if (m_editMode == EditConnectionPoint) {
        KoShape *hoverShape = findNonConnectionShapeAtPosition(event->point); // TODO exclude connectors, need snap guide maybe?
        if (hoverShape) {
            m_currentShape = hoverShape;
            Q_ASSERT(m_currentShape);
            // check if we should highlight another connection point
            int handle = handleAtPoint(m_currentShape, event->point);
            if (handle >= 0) {
                setEditMode(m_editMode, m_currentShape, handle);
                useCursor(handle >= KoConnectionPoint::FirstCustomConnectionPoint ? Qt::SizeAllCursor : Qt::ArrowCursor);
            } else {
                updateStatusText();
                useCursor(Qt::CrossCursor);
            }
        } else {
            m_currentShape = nullptr;
            useCursor(Qt::ArrowCursor);
        }
    } else if (m_editMode == EditConnection) {
        Q_ASSERT(m_currentShape);
        KoShape *hoverShape = findShapeAtPosition(event->point);
        // check if we should highlight another connection handle
        int handle = handleAtPoint(m_currentShape, event->point);
        setEditMode(m_editMode, m_currentShape, handle);
        if (m_activeHandle == KoConnectionShape::StartHandle || m_activeHandle == KoConnectionShape::EndHandle) {
            useCursor(Qt::SizeAllCursor);
        } else if (m_activeHandle >= KoConnectionShape::ControlHandle_1) {
        } else if (hoverShape && hoverShape != m_currentShape) {
            useCursor(Qt::PointingHandCursor);
        } else {
            useCursor(Qt::ArrowCursor);
        }
    } else { // Idle and no current strategy
        KoShape *hoverShape = findShapeAtPosition(event->point);
        int hoverHandle = -1;
        if (hoverShape) {
            KoConnectionShape *connectionShape = dynamic_cast<KoConnectionShape *>(hoverShape);
            if (!connectionShape) {
                QPointF snappedPos = canvas()->snapGuide()->snap(event->point, event->modifiers());
                hoverHandle = handleAtPoint(hoverShape, snappedPos);
                setEditMode(hoverHandle >= 0 ? CreateConnection : Idle, hoverShape, hoverHandle);
            }
            useCursor(hoverHandle >= 0 ? m_connectCursor : Qt::PointingHandCursor);
        } else {
            useCursor(Qt::ArrowCursor);
        }
    }
}

void ConnectionTool::mouseReleaseEvent(KoPointerEvent *event)
{
    if (m_currentStrategy) {
        KUndo2Command *createCmd = nullptr;
        if (m_editMode == CreateConnection) {
            // check if connection handles have a minimal distance
            KoConnectionShape *connectionShape = dynamic_cast<KoConnectionShape *>(m_currentShape);
            Q_ASSERT(connectionShape);
            // get both handle positions in document coordinates
            QPointF p1 = connectionShape->shapeToDocument(connectionShape->handlePosition(0));
            QPointF p2 = connectionShape->shapeToDocument(connectionShape->handlePosition(1));
            int grabDistance = grabSensitivity();
            // use grabbing sensitivity as minimal distance threshold
            if (squareDistance(p1, p2) < grabDistance * grabDistance) {
                // minimal distance was not reached, so we have to undo the started work
                canvas()->shapeManager()->remove(m_currentShape); // deactivate is called
                delete connectionShape;
                return;
            } else {
                // finalize adding the new connection shape with an undo command
                KUndo2Command *cmd = canvas()->shapeController()->addShape(m_currentShape);
                setEditMode(EditConnection, m_currentShape, KoConnectionShape::StartHandle);

                createCmd = new KUndo2Command(kundo2_i18n("Create Connection"));
                createCmd->addCommand(cmd);
            }
        }
        m_currentStrategy->finishInteraction(event->modifiers());
        KUndo2Command *command = m_currentStrategy->createCommand();
        if (command) {
            if (createCmd) {
                createCmd->addCommand(command);
                canvas()->addCommand(createCmd);
            } else {
                canvas()->addCommand(command);
            }
        } else {
            delete createCmd;
            if (m_editMode == CreateConnection) {
                // creation failed, so we have to undo the started work
                KoConnectionShape *connectionShape = dynamic_cast<KoConnectionShape *>(m_currentShape);
                Q_ASSERT(connectionShape);
                canvas()->shapeManager()->remove(m_currentShape); // deactivate is called
                delete connectionShape;
                return;
            }
        }
        delete m_currentStrategy;
        m_currentStrategy = nullptr;
    }
    updateStatusText();
}

void ConnectionTool::mouseDoubleClickEvent(KoPointerEvent *event)
{
    if (m_editMode == EditConnectionPoint) {
        repaintDecorations();

        // quit EditConnectionPoint mode when double click blank region on canvas
        if (!m_currentShape) {
            resetEditMode();
            return;
        }

        // add connection point when double click a shape
        // remove connection point when double click a existed connection point
        int handleId = handleAtPoint(m_currentShape, event->point);
        if (handleId < 0) {
            QPointF mousePos = canvas()->snapGuide()->snap(event->point, event->modifiers());
            QPointF point = m_currentShape->documentToShape(mousePos);
            canvas()->addCommand(new AddConnectionPointCommand(m_currentShape, point));
        } else {
            canvas()->addCommand(new RemoveConnectionPointCommand(m_currentShape, handleId));
        }
        setEditMode(m_editMode, m_currentShape, -1);
    } else {
        // deactivate connection tool when double click blank region on canvas
        KoShape *hitShape = findShapeAtPosition(event->point);
        if (!hitShape) {
            deactivate();
            Q_EMIT done();
        } else if (dynamic_cast<KoConnectionShape *>(hitShape)) {
            repaintDecorations();
            setEditMode(EditConnection, m_currentShape, -1);
            // TODO: temporarily activate text tool to edit connection path
        }
    }
}

void ConnectionTool::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        deactivate();
        Q_EMIT done();
    } else if (event->key() == Qt::Key_Backspace) {
        deleteSelection();
        event->accept();
    }
}

void ConnectionTool::slotShapeRemoved(KoShape *shape)
{
    if (m_currentShape && m_currentShape == shape) {
        deactivate();
    }
}

void ConnectionTool::activate(ToolActivation, const QSet<KoShape *> &)
{
    // save old enabled snap strategies, set bounding box snap strategy
    m_oldSnapStrategies = canvas()->snapGuide()->enabledSnapStrategies();
    canvas()->snapGuide()->enableSnapStrategies(KoSnapGuide::BoundingBoxSnapping);
    canvas()->snapGuide()->reset();
    m_resetPaint = true;
    repaintDecorations();
}

void ConnectionTool::deactivate()
{
    // Put everything to 0 to be able to begin a new shape properly
    delete m_currentStrategy;
    m_currentStrategy = nullptr;
    resetEditMode();
    m_resetPaint = true;
    repaintDecorations();
    // restore previously set snap strategies
    canvas()->snapGuide()->enableSnapStrategies(m_oldSnapStrategies);
    canvas()->snapGuide()->reset();
    m_currentShape = nullptr;
}

qreal ConnectionTool::squareDistance(const QPointF &p1, const QPointF &p2) const
{
    // Square of the distance
    const qreal dx = p2.x() - p1.x();
    const qreal dy = p2.y() - p1.y();
    return dx * dx + dy * dy;
}

KoShape *ConnectionTool::findShapeAtPosition(const QPointF &position) const
{
    QList<KoShape *> shapes = canvas()->shapeManager()->shapesAt(handleGrabRect(position));
    if (!shapes.isEmpty()) {
        std::sort(shapes.begin(), shapes.end(), KoShape::compareShapeZIndex);
        // we want to priorize connection shape handles, even if the connection shape
        // is not at the top of the shape stack at the mouse position
        KoConnectionShape *connectionShape = nearestConnectionShape(shapes, position);
        // use best connection shape or first shape from stack (last in the list) if not found
        if (connectionShape) {
            return connectionShape;
        } else {
            for (QList<KoShape *>::const_iterator end = shapes.constEnd() - 1; end >= shapes.constBegin(); --end) {
                KoShape *shape = *end;
                if (!dynamic_cast<KoConnectionShape *>(shape) && shape->shapeId() != TextShape_SHAPEID) {
                    return shape;
                }
            }
        }
    }

    return nullptr;
}

KoShape *ConnectionTool::findNonConnectionShapeAtPosition(const QPointF &position) const
{
    QList<KoShape *> shapes = canvas()->shapeManager()->shapesAt(handleGrabRect(position));
    if (!shapes.isEmpty()) {
        std::sort(shapes.begin(), shapes.end(), KoShape::compareShapeZIndex);
        for (QList<KoShape *>::const_iterator end = shapes.constEnd() - 1; end >= shapes.constBegin(); --end) {
            KoShape *shape = *end;
            if (!dynamic_cast<KoConnectionShape *>(shape) && shape->shapeId() != TextShape_SHAPEID) {
                return shape;
            }
        }
    }

    return nullptr;
}

int ConnectionTool::handleAtPoint(KoShape *shape, const QPointF &mousePoint) const
{
    if (!shape)
        return -1;

    const QPointF shapePoint = shape->documentToShape(mousePoint);

    KoConnectionShape *connectionShape = dynamic_cast<KoConnectionShape *>(shape);
    if (connectionShape) {
        // check connection shape handles
        return connectionShape->handleIdAt(handleGrabRect(shapePoint));
    } else {
        // check connection points
        int grabDistance = grabSensitivity();
        qreal minDistance = HUGE_VAL;
        int handleId = -1;
        KoConnectionPoints connectionPoints = shape->connectionPoints();
        KoConnectionPoints::const_iterator cp = connectionPoints.constBegin();
        KoConnectionPoints::const_iterator lastCp = connectionPoints.constEnd();
        for (; cp != lastCp; ++cp) {
            qreal d = squareDistance(shapePoint, cp.value().position);
            if (d <= grabDistance && d < minDistance) {
                handleId = cp.key();
                minDistance = d;
            }
        }
        return handleId;
    }
}

KoConnectionShape *ConnectionTool::nearestConnectionShape(const QList<KoShape *> &shapes, const QPointF &mousePos) const
{
    int grabDistance = grabSensitivity();

    KoConnectionShape *nearestConnectionShape = nullptr;
    qreal minSquaredDistance = HUGE_VAL;
    const qreal maxSquaredDistance = grabDistance * grabDistance;

    foreach (KoShape *shape, shapes) {
        KoConnectionShape *connectionShape = dynamic_cast<KoConnectionShape *>(shape);
        if (!connectionShape || !connectionShape->isParametricShape())
            continue;

        // convert document point to shape coordinates
        QPointF p = connectionShape->documentToShape(mousePos);
        // our region of interest, i.e. a region around our mouse position
        QRectF roi = handleGrabRect(p);

        // check all segments of this shape which intersect the region of interest
        QList<KoPathSegment> segments = connectionShape->segmentsAt(roi);
        foreach (const KoPathSegment &s, segments) {
            qreal nearestPointParam = s.nearestPoint(p);
            QPointF nearestPoint = s.pointAt(nearestPointParam);
            QPointF diff = p - nearestPoint;
            qreal squaredDistance = diff.x() * diff.x() + diff.y() * diff.y();
            // are we within the allowed distance ?
            if (squaredDistance > maxSquaredDistance)
                continue;
            // are we closer to the last closest point ?
            if (squaredDistance < minSquaredDistance) {
                nearestConnectionShape = connectionShape;
                minSquaredDistance = squaredDistance;
            }
        }
    }

    return nearestConnectionShape;
}

void ConnectionTool::setEditMode(EditMode mode, KoShape *currentShape, int handle)
{
    repaintDecorations();
    m_editMode = mode;
    if (m_currentShape != currentShape) {
        KoConnectionShape *connectionShape = dynamic_cast<KoConnectionShape *>(currentShape);
        foreach (KoShapeConfigWidgetBase *cw, m_connectionShapeWidgets) {
            if (connectionShape)
                cw->open(currentShape);
        }
    }
    if (mode == Idle) {
        Q_EMIT sendConnectionType(m_connectionType);
    }
    m_currentShape = currentShape;
    m_activeHandle = handle;
    repaintDecorations();
    updateActions();
    updateStatusText();
}

void ConnectionTool::resetEditMode()
{
    m_connectionType = KoConnectionShape::Standard;
    setEditMode(Idle, nullptr, -1);
    Q_EMIT sendConnectionPointEditState(false);
}

void ConnectionTool::updateActions()
{
    const bool connectionPointSelected = m_editMode == EditConnectionPoint && m_activeHandle >= 0;
    if (connectionPointSelected) {
        KoConnectionPoint cp = m_currentShape->connectionPoint(m_activeHandle);

        m_alignPercent->setChecked(false);
        foreach (QAction *action, m_alignHorizontal->actions())
            action->setChecked(false);
        foreach (QAction *action, m_alignVertical->actions())
            action->setChecked(false);
        switch (cp.alignment) {
        case KoConnectionPoint::AlignNone:
            m_alignPercent->setChecked(true);
            break;
        case KoConnectionPoint::AlignTopLeft:
            m_alignLeft->setChecked(true);
            m_alignTop->setChecked(true);
            break;
        case KoConnectionPoint::AlignTop:
            m_alignCenterH->setChecked(true);
            m_alignTop->setChecked(true);
            break;
        case KoConnectionPoint::AlignTopRight:
            m_alignRight->setChecked(true);
            m_alignTop->setChecked(true);
            break;
        case KoConnectionPoint::AlignLeft:
            m_alignLeft->setChecked(true);
            m_alignCenterV->setChecked(true);
            break;
        case KoConnectionPoint::AlignCenter:
            m_alignCenterH->setChecked(true);
            m_alignCenterV->setChecked(true);
            break;
        case KoConnectionPoint::AlignRight:
            m_alignRight->setChecked(true);
            m_alignCenterV->setChecked(true);
            break;
        case KoConnectionPoint::AlignBottomLeft:
            m_alignLeft->setChecked(true);
            m_alignBottom->setChecked(true);
            break;
        case KoConnectionPoint::AlignBottom:
            m_alignCenterH->setChecked(true);
            m_alignBottom->setChecked(true);
            break;
        case KoConnectionPoint::AlignBottomRight:
            m_alignRight->setChecked(true);
            m_alignBottom->setChecked(true);
            break;
        }
        foreach (QAction *action, m_escapeDirections->actions())
            action->setChecked(false);
        switch (cp.escapeDirection) {
        case KoConnectionPoint::AllDirections:
            m_escapeAll->setChecked(true);
            break;
        case KoConnectionPoint::HorizontalDirections:
            m_escapeHorizontal->setChecked(true);
            break;
        case KoConnectionPoint::VerticalDirections:
            m_escapeVertical->setChecked(true);
            break;
        case KoConnectionPoint::LeftDirection:
            m_escapeLeft->setChecked(true);
            break;
        case KoConnectionPoint::RightDirection:
            m_escapeRight->setChecked(true);
            break;
        case KoConnectionPoint::UpDirection:
            m_escapeUp->setChecked(true);
            break;
        case KoConnectionPoint::DownDirection:
            m_escapeDown->setChecked(true);
            break;
        }
    }
    Q_EMIT connectionPointEnabled(connectionPointSelected);
}

void ConnectionTool::updateStatusText()
{
    switch (m_editMode) {
    case Idle:
        if (m_currentShape) {
            if (dynamic_cast<KoConnectionShape *>(m_currentShape)) {
                if (m_activeHandle >= 0)
                    Q_EMIT statusTextChanged(i18n("Drag to edit connection."));
                else
                    Q_EMIT statusTextChanged(i18n("Double click connection or press delete to remove it."));
            } else if (m_activeHandle < 0) {
                Q_EMIT statusTextChanged(i18n("Click to edit connection points."));
            }
        } else {
            Q_EMIT statusTextChanged("");
        }
        break;
    case EditConnection:
        if (m_activeHandle >= 0)
            Q_EMIT statusTextChanged(i18n("Drag to edit connection."));
        else
            Q_EMIT statusTextChanged(i18n("Double click connection or press delete to remove it."));
        break;
    case EditConnectionPoint:
        if (m_activeHandle >= KoConnectionPoint::FirstCustomConnectionPoint)
            Q_EMIT statusTextChanged(i18n("Drag to move connection point. Double click connection or press delete to remove it."));
        else if (m_activeHandle >= 0)
            Q_EMIT statusTextChanged(i18n("Double click connection point or press delete to remove it."));
        else
            Q_EMIT statusTextChanged(i18n("Double click to add connection point."));
        break;
    case CreateConnection:
        Q_EMIT statusTextChanged(i18n("Drag to create new connection."));
        break;
    default:
        Q_EMIT statusTextChanged("");
    }
}

QList<QPointer<QWidget>> ConnectionTool::createOptionWidgets()
{
    QList<QPointer<QWidget>> list;

    m_connectionShapeWidgets.clear();

    KoShapeFactoryBase *factory = KoShapeRegistry::instance()->get(KOCONNECTIONSHAPEID);
    if (factory) {
        QList<KoShapeConfigWidgetBase *> widgets = factory->createShapeOptionPanels();
        foreach (KoShapeConfigWidgetBase *cw, widgets) {
            if (cw->showOnShapeCreate() || !cw->showOnShapeSelect()) {
                delete cw;
                continue;
            }
            connect(cw, &KoShapeConfigWidgetBase::propertyChanged, this, &ConnectionTool::connectionChanged);
            KoConnectionShapeConfigWidget *cw2 = (KoConnectionShapeConfigWidget *)cw;
            if (cw2) {
                connect(cw2, &KoConnectionShapeConfigWidget::connectionTypeChanged, this, &ConnectionTool::getConnectionType);
                connect(this, &ConnectionTool::sendConnectionType, cw2, &KoConnectionShapeConfigWidget::setConnectionType);
            }
            m_connectionShapeWidgets.append(cw);
            cw->setWindowTitle(i18n("Connection"));
            list.append(cw);
        }
    }
    KoStrokeConfigWidget *strokeWidget = new KoStrokeConfigWidget(nullptr);
    strokeWidget->setWindowTitle(i18n("Line"));
    strokeWidget->setCanvas(canvas());
    list.append(strokeWidget);

    ConnectionPointWidget *connectPoint = new ConnectionPointWidget(this);
    connectPoint->setWindowTitle(i18n("Connection Point"));
    list.append(connectPoint);

    return list;
}

void ConnectionTool::horizontalAlignChanged()
{
    if (m_alignPercent->isChecked()) {
        m_alignPercent->setChecked(false);
        m_alignTop->setChecked(true);
    }
    updateConnectionPoint();
}

void ConnectionTool::verticalAlignChanged()
{
    if (m_alignPercent->isChecked()) {
        m_alignPercent->setChecked(false);
        m_alignLeft->setChecked(true);
    }
    updateConnectionPoint();
}

void ConnectionTool::relativeAlignChanged()
{
    foreach (QAction *action, m_alignHorizontal->actions())
        action->setChecked(false);
    foreach (QAction *action, m_alignVertical->actions())
        action->setChecked(false);
    m_alignPercent->setChecked(true);

    updateConnectionPoint();
}

void ConnectionTool::updateConnectionPoint()
{
    if (m_editMode == EditConnectionPoint && m_currentShape && m_activeHandle >= 0) {
        KoConnectionPoint oldPoint = m_currentShape->connectionPoint(m_activeHandle);
        KoConnectionPoint newPoint = oldPoint;
        if (m_alignPercent->isChecked()) {
            newPoint.alignment = KoConnectionPoint::AlignNone;
        } else if (m_alignLeft->isChecked() && m_alignTop->isChecked()) {
            newPoint.alignment = KoConnectionPoint::AlignTopLeft;
        } else if (m_alignCenterH->isChecked() && m_alignTop->isChecked()) {
            newPoint.alignment = KoConnectionPoint::AlignTop;
        } else if (m_alignRight->isChecked() && m_alignTop->isChecked()) {
            newPoint.alignment = KoConnectionPoint::AlignTopRight;
        } else if (m_alignLeft->isChecked() && m_alignCenterV->isChecked()) {
            newPoint.alignment = KoConnectionPoint::AlignLeft;
        } else if (m_alignCenterH->isChecked() && m_alignCenterV->isChecked()) {
            newPoint.alignment = KoConnectionPoint::AlignCenter;
        } else if (m_alignRight->isChecked() && m_alignCenterV->isChecked()) {
            newPoint.alignment = KoConnectionPoint::AlignRight;
        } else if (m_alignLeft->isChecked() && m_alignBottom->isChecked()) {
            newPoint.alignment = KoConnectionPoint::AlignBottomLeft;
        } else if (m_alignCenterH->isChecked() && m_alignBottom->isChecked()) {
            newPoint.alignment = KoConnectionPoint::AlignBottom;
        } else if (m_alignRight->isChecked() && m_alignBottom->isChecked()) {
            newPoint.alignment = KoConnectionPoint::AlignBottomRight;
        }

        canvas()->addCommand(new ChangeConnectionPointCommand(m_currentShape, m_activeHandle, oldPoint, newPoint));
    }
}

void ConnectionTool::escapeDirectionChanged()
{
    if (m_editMode == EditConnectionPoint && m_currentShape && m_activeHandle >= 0) {
        KoConnectionPoint oldPoint = m_currentShape->connectionPoint(m_activeHandle);
        KoConnectionPoint newPoint = oldPoint;
        QAction *checkedAction = m_escapeDirections->checkedAction();
        if (checkedAction == m_escapeAll) {
            newPoint.escapeDirection = KoConnectionPoint::AllDirections;
        } else if (checkedAction == m_escapeHorizontal) {
            newPoint.escapeDirection = KoConnectionPoint::HorizontalDirections;
        } else if (checkedAction == m_escapeVertical) {
            newPoint.escapeDirection = KoConnectionPoint::VerticalDirections;
        } else if (checkedAction == m_escapeLeft) {
            newPoint.escapeDirection = KoConnectionPoint::LeftDirection;
        } else if (checkedAction == m_escapeRight) {
            newPoint.escapeDirection = KoConnectionPoint::RightDirection;
        } else if (checkedAction == m_escapeUp) {
            newPoint.escapeDirection = KoConnectionPoint::UpDirection;
        } else if (checkedAction == m_escapeDown) {
            newPoint.escapeDirection = KoConnectionPoint::DownDirection;
        }
        canvas()->addCommand(new ChangeConnectionPointCommand(m_currentShape, m_activeHandle, oldPoint, newPoint));
    }
}

void ConnectionTool::connectionChanged()
{
    if (m_editMode != EditConnection) {
        return;
    }
    KoConnectionShape *connectionShape = dynamic_cast<KoConnectionShape *>(m_currentShape);
    if (!connectionShape)
        return;

    foreach (KoShapeConfigWidgetBase *cw, m_connectionShapeWidgets) {
        canvas()->addCommand(cw->createCommand());
    }
}

void ConnectionTool::deleteSelection()
{
    if (m_editMode == EditConnectionPoint && m_currentShape && m_activeHandle >= 0) {
        repaintDecorations();
        canvas()->addCommand(new RemoveConnectionPointCommand(m_currentShape, m_activeHandle));
        setEditMode(m_editMode, m_currentShape, -1);
    } else if (m_editMode == EditConnection && m_currentShape) {
        repaintDecorations();
        canvas()->addCommand(canvas()->shapeController()->removeShape(m_currentShape));
        resetEditMode();
    }
}

void ConnectionTool::getConnectionType(int type)
{
    if (m_editMode == Idle)
        m_connectionType = (KoConnectionShape::Type)type;
}

void ConnectionTool::toggleConnectionPointEditMode(int state)
{
    if (state == Qt::Checked)
        setEditMode(EditConnectionPoint, nullptr, -1);
    else if (state == Qt::Unchecked)
        setEditMode(Idle, nullptr, -1);
    else
        return;
}
