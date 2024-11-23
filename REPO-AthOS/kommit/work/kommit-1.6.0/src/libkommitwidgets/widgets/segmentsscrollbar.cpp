/*
SPDX-FileCopyrightText: 2021 Hamed Masafi <hamed.masfi@gmail.com>

SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "segmentsscrollbar.h"

#include "gitglobal.h"
#include "segmentconnector.h"
#include "widgets/codeeditor.h"

#include <QPaintEvent>
#include <QPainter>
#include <kommitwidgetsglobaloptions.h>

SegmentsScrollBar::SegmentsScrollBar(QWidget *parent)
    : QWidget{parent}
{
    setMouseTracking(true);
}

SegmentConnector *SegmentsScrollBar::segmentConnector() const
{
    return mSegmentConnector;
}

void SegmentsScrollBar::setSegmentConnector(SegmentConnector *newSegmentConnector)
{
    if (mSegmentConnector) {
        disconnect(newSegmentConnector, &SegmentConnector::segmentsChanged, this, &SegmentsScrollBar::reload);
        disconnect(newSegmentConnector, &SegmentConnector::sameSizeChanged, this, &SegmentsScrollBar::reload);
    }
    mSegmentConnector = newSegmentConnector;
    connect(newSegmentConnector, &SegmentConnector::segmentsChanged, this, &SegmentsScrollBar::reload);
    connect(newSegmentConnector, &SegmentConnector::sameSizeChanged, this, &SegmentsScrollBar::reload);
    update();
}

void SegmentsScrollBar::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.fillRect(event->rect(), Qt::white);

    if (!Q_UNLIKELY(mSegmentConnector))
        return;

    painter.drawImage(0, 0, mSegmentsImage);

    auto leftArea = mSegmentConnector->left()->visibleLines();
    auto rightArea = mSegmentConnector->right()->visibleLines();

    QBrush br(Qt::gray);

    painter.fillRect(QRect{int(width() * .4), 0, int(width() * .2), height() - 1}, palette().base());
    painter.drawLine(width() * .4, 0, width() * .4, height() - 1);
    painter.drawLine(width() * .6, 0, width() * .6, height() - 1);

    painter.setOpacity(.3);
    paintSection(painter, Left, leftArea.first, leftArea.second, br, true);
    paintSection(painter, Right, rightArea.first, rightArea.second, br, true);
    painter.setOpacity(1);
    painter.drawRect(QRect{0, 0, width() - 1, height() - 1});
}

void SegmentsScrollBar::mouseMoveEvent(QMouseEvent *event)
{
    QWidget::mouseMoveEvent(event);
    Q_EMIT hover(event->y(), static_cast<double>(event->y()) / static_cast<double>(height()));
}

void SegmentsScrollBar::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
    if (Q_LIKELY(mSegmentConnector))
        generateSegmentsImage();
}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
void SegmentsScrollBar::enterEvent(QEvent *event)
#else
void SegmentsScrollBar::enterEvent(QEnterEvent *event)
#endif
{
    QWidget::enterEvent(event);
    Q_EMIT mouseEntered();
}

void SegmentsScrollBar::leaveEvent(QEvent *event)
{
    QWidget::leaveEvent(event);
    Q_EMIT mouseLeaved();
}

void SegmentsScrollBar::reload()
{
    leftCount = rightCount = 0;
    for (const auto &segment : qAsConst(mSegmentConnector->segments())) {
        if (mSegmentConnector->sameSize()) {
            auto m = qMax(segment->oldText.size(), segment->newText.size());
            leftCount += m;
            rightCount += m;
        } else {
            leftCount += segment->oldText.size();
            rightCount += segment->newText.size();
        }
    }
    update();
}

void SegmentsScrollBar::generateSegmentsImage()
{
    mSegmentsImage = QImage{width(), height(), QImage::Format_ARGB32};

    mSegmentsImage.fill(Qt::white);
    QPainter painter{&mSegmentsImage};

    int countLeft{0};
    int countRight{0};

    for (const auto &segment : qAsConst(mSegmentConnector->segments())) {
        QBrush brush;
        switch (segment->type) {
        case Diff::SegmentType::OnlyOnLeft:
            brush = KommitWidgetsGlobalOptions::instance()->statucColor(Git::ChangeStatus::Removed);
            break;
        case Diff::SegmentType::OnlyOnRight:
            brush = KommitWidgetsGlobalOptions::instance()->statucColor(Git::ChangeStatus::Added);
            break;
        case Diff::SegmentType::SameOnBoth:
            brush = Qt::white;
            break;
        case Diff::SegmentType::DifferentOnBoth:
            brush = KommitWidgetsGlobalOptions::instance()->statucColor(Git::ChangeStatus::Modified);
            break;
        default:
            break;
        }

        paintSection(painter, Left, countLeft, segment->oldText.size(), brush);
        paintSection(painter, Right, countRight, segment->newText.size(), brush);

        if (mSegmentConnector->sameSize()) {
            auto m = qMax(segment->oldText.size(), segment->newText.size());

            paintSection(painter, Left, countLeft + segment->oldText.size(), m - segment->oldText.size(), Qt::darkGray);
            paintSection(painter, Right, countRight + segment->newText.size(), m - segment->newText.size(), Qt::darkGray);

            countLeft += m;
            countRight += m;
        } else {
            countLeft += segment->oldText.size();
            countRight += segment->newText.size();
        }
    }
    /*
        auto leftArea = mSegmentConnector->left()->visibleLines();
        auto rightArea = mSegmentConnector->right()->visibleLines();

        QBrush br(Qt::gray);

        painter.fillRect(QRect{int(width() * .4), 0, int(width() * .2), height() - 1}, palette().base());
        painter.drawLine(width() * .4, 0, width() * .4, height() - 1);
        painter.drawLine(width() * .6, 0, width() * .6, height() - 1);

        painter.setOpacity(.3);
        paintSection(painter, Left, leftArea.first, leftArea.second, br, true);
        paintSection(painter, Right, rightArea.first, rightArea.second, br, true);
        painter.setOpacity(1);
        painter.drawRect(QRect{0, 0, width() - 1, height() - 1});*/
}

void SegmentsScrollBar::paintSection(QPainter &painter, SegmentsScrollBar::Side side, int from, int len, const QBrush &brush, bool drawRect)
{
    if (len <= 0)
        return;
    int w = width() * .4;
    int l = side == Left ? 0 : width() - w;
    int &c = side == Left ? leftCount : rightCount;

    QRect rc{l, static_cast<int>((from / (double)c) * height()) - 1, w, static_cast<int>((len / (double)c) * height())};
    painter.fillRect(rc, brush);
    if (drawRect)
        painter.drawRect(rc);
}

#include "moc_segmentsscrollbar.cpp"
