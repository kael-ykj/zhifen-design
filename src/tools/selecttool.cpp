#include "selecttool.h"
#include "cadview.h"
#include "cadscene.h"
#include "caditem.h"
#include <QMouseEvent>
#include <QPainter>

SelectTool::SelectTool(CadView *view, QObject *parent)
    : Tool(view, parent)
{
}

void SelectTool::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) return;

    m_startScreen = event->pos();
    m_currentScreen = event->pos();
    m_selecting = true;

    QPointF worldPos = m_view->mapToScene(event->pos());
    m_lastWorldPos = worldPos;

    // 检查是否点击在已选中的图元上（移动模式）
    if (m_scene) {
        QGraphicsItem *item = m_scene->itemAt(worldPos, m_view->transform());
        CadItem *cadItem = dynamic_cast<CadItem*>(item);
        if (cadItem && cadItem->isSelected()) {
            m_moveMode = true;
            m_moveStart = worldPos;
            return;
        }
    }

    // 清除选择（非Shift时）
    if (!(event->modifiers() & Qt::ShiftModifier) && m_scene) {
        m_scene->clearSelection();
    }
}

void SelectTool::mouseMoveEvent(QMouseEvent *event)
{
    m_currentScreen = event->pos();
    QPointF worldPos = m_view->mapToScene(event->pos());
    m_lastWorldPos = worldPos;

    if (m_moveMode && m_scene) {
        QPointF delta = worldPos - m_moveStart;
        for (auto item : m_scene->selectedItems()) {
            item->moveBy(delta.x(), delta.y());
        }
        m_moveStart = worldPos;
    }
}

void SelectTool::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) return;

    if (m_moveMode) {
        m_moveMode = false;
        m_selecting = false;
        return;
    }

    if (m_selecting && m_scene) {
        QPointF worldStart = m_view->mapToScene(m_startScreen);
        QPointF worldEnd = m_view->mapToScene(m_currentScreen);
        QRectF selectRect = QRectF(worldStart, worldEnd).normalized();

        // 单击选择
        if (selectRect.width() < 3 && selectRect.height() < 3) {
            QGraphicsItem *item = m_scene->itemAt(worldEnd, m_view->transform());
            if (item) {
                if (event->modifiers() & Qt::ShiftModifier) {
                    item->setSelected(!item->isSelected());
                } else {
                    m_scene->clearSelection();
                    item->setSelected(true);
                }
            }
        } else {
            // 框选
            bool intersect = m_startScreen.x() > m_currentScreen.x(); // 从右向左=交叉
            QPainterPath path;
            path.addRect(selectRect);
            if (intersect) {
                m_scene->setSelectionArea(path, Qt::IntersectsItemShape, m_view->transform());
            } else {
                m_scene->setSelectionArea(path, Qt::ContainsItemShape, m_view->transform());
            }
        }
    }
    m_selecting = false;
}

void SelectTool::drawOverlay(QPainter *painter)
{
    if (m_selecting && !m_moveMode) {
        QRectF rect = QRectF(m_startScreen, m_currentScreen).normalized();
        bool intersect = m_startScreen.x() > m_currentScreen.x();
        painter->save();
        painter->setPen(QPen(intersect ? QColor(0, 255, 255) : QColor(0, 255, 0), 1, Qt::DashLine));
        painter->setBrush(QColor(intersect ? 0 : 0, 255, 0, 30));
        painter->drawRect(rect);
        painter->restore();
    }
}
