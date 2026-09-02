#include "cadview.h"
#include "cadscene.h"
#include "tool.h"
#include "snapmanager.h"
#include "gripmanager.h"
#include "dynamicinput.h"
#include "blocks/blockreference.h"
#include "document.h"
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QScrollBar>
#include <QPainter>
#include <QtMath>

CadView::CadView(QWidget *parent)
    : QGraphicsView(parent)
{
    setRenderHint(QPainter::Antialiasing, true);
    setRenderHint(QPainter::SmoothPixmapTransform, true);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setDragMode(QGraphicsView::NoDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorViewCenter);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setBackgroundBrush(QColor(0, 0, 0));
}

void CadView::setCadScene(CadScene *scene)
{
    setScene(scene);
    m_snapManager = new SnapManager(scene, this);
    m_gripManager = new Zhifen::GripManager(scene, this);
    connect(scene, &CadScene::selectionChanged, this, [this](){ if (m_gripManager) m_gripManager->updateGrips(); });
    m_dynamicInput = new DynamicInput(this);
}

void CadView::zoomExtents()
{
    if (!scene()) return;
    QRectF rect = scene()->itemsBoundingRect();
    if (rect.isEmpty()) rect = QRectF(-100, -100, 200, 200);
    rect.adjust(-rect.width() * 0.1, -rect.height() * 0.1,
                rect.width() * 0.1, rect.height() * 0.1);
    fitInView(rect, Qt::KeepAspectRatio);
}

void CadView::zoomIn()
{
    scale(1.2, 1.2);
}

void CadView::zoomOut()
{
    scale(1 / 1.2, 1 / 1.2);
}

void CadView::zoomAt(const QPoint &screenPos, qreal factor)
{
    QPointF worldPos = mapToScene(screenPos);
    scale(factor, factor);
    QPointF newWorldPos = mapToScene(screenPos);
    QPointF delta = newWorldPos - worldPos;
    translate(delta.x(), delta.y());
}

QPointF CadView::screenToWorld(const QPoint &screenPos) const
{
    return mapToScene(screenPos);
}

QPoint CadView::worldToScreen(const QPointF &worldPos) const
{
    return mapFromScene(worldPos);
}

void CadView::mousePressEvent(QMouseEvent *event)
{
    // 隐藏动态输入
    if (m_dynamicInput && m_dynamicInput->isVisible()) {
        m_dynamicInput->hideInput();
    }

    // 中键平移
    if (event->button() == Qt::MiddleButton) {
        m_panning = true;
        m_panStart = event->pos();
        setCursor(Qt::ClosedHandCursor);
        return;
    }

    // 左键点击夹点
    if (event->button() == Qt::LeftButton && m_gripManager && m_gripManager->hasGrips()) {
        QPointF worldPos = mapToScene(event->pos());
        Zhifen::GripPoint *grip = m_gripManager->gripAt(worldPos);
        if (grip) {
            m_gripDragging = true;
            m_draggedGrip = grip;
            m_gripManager->setHotGrip(grip);
            setCursor(Qt::SizeAllCursor);
            return;
        }
    }

    if (m_currentTool) {
        m_currentTool->mousePressEvent(event);
    } else {
        // 左键点击场景（用于器件放置等）
        if (event->button() == Qt::LeftButton) {
            emit sceneClicked(mapToScene(event->pos()));
        }
        QGraphicsView::mousePressEvent(event);
    }
}

void CadView::mouseMoveEvent(QMouseEvent *event)
{
    QPointF worldPos = mapToScene(event->pos());

    // 对象捕捉计算
    if (m_snapManager && m_snapManager->isEnabled() && !m_panning) {
        SnapResult *snap = m_snapManager->computeSnap(worldPos);
        if (snap) {
            worldPos = snap->point;
        }
    }

    m_lastWorldPos = worldPos;
    emit coordinateChanged(worldPos);

    // 动态输入
    if (m_dynamicInput && m_dynamicInputEnabled && m_currentTool) {
        m_dynamicInput->updateCoordinate(worldPos);
        m_dynamicInput->updatePosition(event->globalPos());
        if (!m_dynamicInput->isVisible()) m_dynamicInput->showInput();
    }

    // 中键平移
    if (m_panning) {
        QPoint delta = event->pos() - m_panStart;
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());
        m_panStart = event->pos();
        return;
    }

    // 夹点拖拽
    if (m_gripDragging && m_draggedGrip && m_gripManager) {
        // 正交模式
        if (m_orthoMode && m_draggedGrip->type == Zhifen::Grip_Endpoint) {
            QPointF start = m_draggedGrip->position;
            QPointF delta = worldPos - start;
            if (qAbs(delta.x()) > qAbs(delta.y())) {
                worldPos.setY(start.y());
            } else {
                worldPos.setX(start.x());
            }
        }
        m_gripManager->dragGrip(m_draggedGrip, worldPos);
        viewport()->update();
        return;
    }

    // 检测鼠标悬停在夹点上
    if (m_gripManager && m_gripManager->hasGrips() && !m_gripDragging) {
        Zhifen::GripPoint *grip = m_gripManager->gripAt(worldPos);
        if (grip) {
            setCursor(Qt::SizeAllCursor);
        } else {
            setCursor(m_currentTool ? m_currentTool->cursor() : Qt::CrossCursor);
        }
    }

    if (m_currentTool) {
        m_currentTool->mouseMoveEvent(event);
    } else {
        QGraphicsView::mouseMoveEvent(event);
    }
}

void CadView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton && m_panning) {
        m_panning = false;
        setCursor(m_currentTool ? m_currentTool->cursor() : Qt::CrossCursor);
        return;
    }

    // 结束夹点拖拽
    if (event->button() == Qt::LeftButton && m_gripDragging) {
        m_gripDragging = false;
        if (m_gripManager) {
            m_gripManager->clearHotGrip();
            m_gripManager->updateGrips();
        }
        m_draggedGrip = nullptr;
        setCursor(m_currentTool ? m_currentTool->cursor() : Qt::CrossCursor);
        viewport()->update();
        return;
    }

    if (m_currentTool) {
        m_currentTool->mouseReleaseEvent(event);
    } else {
        QGraphicsView::mouseReleaseEvent(event);
    }
}

void CadView::wheelEvent(QWheelEvent *event)
{
    QPoint angle = event->angleDelta();
    qreal factor = angle.y() > 0 ? 1.15 : 1 / 1.15;
    zoomAt(event->pos(), factor);
    event->accept();
}

void CadView::keyPressEvent(QKeyEvent *event)
{
    if (m_currentTool) {
        m_currentTool->keyPressEvent(event);
        if (event->isAccepted()) return;
    }

    switch (event->key()) {
    case Qt::Key_Escape:
        if (m_currentTool) {
            emit toolFinished();
        }
        break;
    case Qt::Key_Delete:
    case Qt::Key_Backspace:
        // 删除选中项
        if (scene()) {
            auto items = scene()->selectedItems();
            for (auto item : items) {
                scene()->removeItem(item);
                delete item;
            }
        }
        break;
    default:
        QGraphicsView::keyPressEvent(event);
    }
}

void CadView::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    menu.setStyleSheet("QMenu { background: #252526; color: #ccc; border: 1px solid #3c3c3c; } QMenu::item:selected { background: #0e639c; } QMenu::separator { background: #3c3c3c; height: 1px; }");

    bool hasSelection = !scene()->selectedItems().isEmpty();

    // 编辑菜单（有选中对象时可用）
    QAction *moveAct = menu.addAction("移动 (M)");
    QAction *copyAct = menu.addAction("复制 (CO)");
    QAction *rotateAct = menu.addAction("旋转 (RO)");
    QAction *scaleAct = menu.addAction("缩放 (SC)");
    QAction *mirrorAct = menu.addAction("镜像 (MI)");
    menu.addSeparator();
    QAction *delAct = menu.addAction("删除 (E)");
    QAction *explodeAct = menu.addAction("分解 (X)");
    moveAct->setEnabled(hasSelection);
    copyAct->setEnabled(hasSelection);
    rotateAct->setEnabled(hasSelection);
    scaleAct->setEnabled(hasSelection);
    mirrorAct->setEnabled(hasSelection);
    delAct->setEnabled(hasSelection);
    explodeAct->setEnabled(hasSelection);
    menu.addSeparator();

    // 剪贴板
    QAction *cutAct = menu.addAction("剪切 (Ctrl+X)");
    QAction *pasteAct = menu.addAction("粘贴 (Ctrl+V)");
    QAction *selectAllAct = menu.addAction("全部选择 (Ctrl+A)");
    cutAct->setEnabled(hasSelection);
    menu.addSeparator();

    // 视图
    QMenu *zoomMenu = menu.addMenu("缩放 (Z)");
    QAction *zoomExtAct = zoomMenu->addAction("全部缩放 (E)");
    QAction *zoomWinAct = zoomMenu->addAction("窗口缩放 (W)");
    QAction *zoomInAct = zoomMenu->addAction("放大");
    QAction *zoomOutAct = zoomMenu->addAction("缩小");
    QAction *panAct = menu.addAction("平移 (P)");
    QAction *regenAct = menu.addAction("重生成 (RE)");
    menu.addSeparator();

    // 绘图辅助
    QAction *gridAct = menu.addAction("切换网格 (F7)");
    QAction *snapAct = menu.addAction("切换捕捉 (F3)");
    QAction *orthoAct = menu.addAction("切换正交 (F8)");
    QAction *osnapAct = menu.addAction("对象捕捉设置...");
    menu.addSeparator();

    // 特性
    QAction *propAct = menu.addAction("特性 (PR)");
    QAction *layerAct = menu.addAction("图层 (LA)");
    propAct->setEnabled(hasSelection);

    QAction *selected = menu.exec(event->globalPos());
    if (!selected) return;

    if (selected == delAct) {
        auto items = scene()->selectedItems();
        for (auto item : items) { scene()->removeItem(item); delete item; }
    } else if (selected == moveAct) {
        emit toolFinished(); // 通知主窗口切换工具
    } else if (selected == copyAct) {
        emit toolFinished();
    } else if (selected == rotateAct) {
        emit toolFinished();
    } else if (selected == scaleAct) {
        emit toolFinished();
    } else if (selected == mirrorAct) {
        emit toolFinished();
    } else if (selected == explodeAct) {
        // 分解组
        auto items = scene()->selectedItems();
        for (auto item : items) {
            if (item->type() == QGraphicsItemGroup::Type) {
                QGraphicsItemGroup *group = static_cast<QGraphicsItemGroup*>(item);
                auto children = group->childItems();
                scene()->destroyItemGroup(group);
                for (auto child : children) {
                    child->setParentItem(nullptr);
                    scene()->addItem(child);
                }
            }
        }
    } else if (selected == cutAct) {
        // 剪切
    } else if (selected == pasteAct) {
        // 粘贴
    } else if (selected == selectAllAct) {
        for (auto item : scene()->items()) item->setSelected(true);
    } else if (selected == zoomExtAct) {
        zoomExtents();
    } else if (selected == zoomWinAct) {
        // 窗口缩放
    } else if (selected == zoomInAct) {
        zoomIn();
    } else if (selected == zoomOutAct) {
        zoomOut();
    } else if (selected == panAct) {
        setDragMode(QGraphicsView::ScrollHandDrag);
    } else if (selected == regenAct) {
        zoomExtents();
        scene()->update();
    } else if (selected == gridAct) {
        if (CadScene *cs = dynamic_cast<CadScene*>(scene())) cs->setShowGrid(!cs->showGrid());
    } else if (selected == snapAct) {
        if (m_snapManager) m_snapManager->setEnabled(!m_snapManager->isEnabled());
    } else if (selected == orthoAct) {
        setOrthoMode(!m_orthoMode);
    } else if (selected == osnapAct) {
        // 对象捕捉设置对话框
    } else if (selected == propAct) {
        // 打开特性面板
    } else if (selected == layerAct) {
        // 打开图层管理
    }

    QGraphicsView::contextMenuEvent(event);
}

void CadView::drawForeground(QPainter *painter, const QRectF &rect)
{
    Q_UNUSED(rect);
    // 绘制大十字光标（标准CAD风格）
    if (m_lastWorldPos.x() >= 0 && m_lastWorldPos.y() >= 0) {
        QPen crossPen(QColor(150, 150, 150), 1, Qt::DashLine);
        painter->setPen(crossPen);
        // 水平线
        painter->drawLine(QPointF(rect.left(), m_lastWorldPos.y()), QPointF(rect.right(), m_lastWorldPos.y()));
        // 垂直线
        painter->drawLine(QPointF(m_lastWorldPos.x(), rect.top()), QPointF(m_lastWorldPos.x(), rect.bottom()));
        // 中心点小方块
        painter->setPen(QPen(QColor(200, 200, 200), 1));
        painter->drawRect(QRectF(m_lastWorldPos.x() - 4, m_lastWorldPos.y() - 4, 8, 8));
    }

    // 绘制UCS图标（左下角）
    qreal ucsSize = 50;
    QPointF ucsOrigin = rect.bottomLeft() + QPointF(ucsSize + 10, -ucsSize - 10);
    painter->setPen(QPen(QColor(200, 50, 50), 2));  // X轴红色
    painter->drawLine(ucsOrigin, ucsOrigin + QPointF(ucsSize, 0));
    painter->setPen(QPen(QColor(50, 200, 50), 2));  // Y轴绿色
    painter->drawLine(ucsOrigin, ucsOrigin + QPointF(0, -ucsSize));
    // 箭头
    painter->setPen(QPen(QColor(200, 50, 50), 2));
    painter->drawLine(ucsOrigin + QPointF(ucsSize, 0), ucsOrigin + QPointF(ucsSize - 8, -4));
    painter->drawLine(ucsOrigin + QPointF(ucsSize, 0), ucsOrigin + QPointF(ucsSize - 8, 4));
    painter->setPen(QPen(QColor(50, 200, 50), 2));
    painter->drawLine(ucsOrigin + QPointF(0, -ucsSize), ucsOrigin + QPointF(-4, -ucsSize + 8));
    painter->drawLine(ucsOrigin + QPointF(0, -ucsSize), ucsOrigin + QPointF(4, -ucsSize + 8));
    // 标签
    painter->setPen(QColor(200, 50, 50));
    painter->setFont(QFont("Arial", 10, QFont::Bold));
    painter->drawText(ucsOrigin + QPointF(ucsSize + 5, 5), "X");
    painter->setPen(QColor(50, 200, 50));
    painter->drawText(ucsOrigin + QPointF(-12, -ucsSize - 5), "Y");

    // 左上角：视口控件/视图控件/视觉样式控件（标准CAD）
    painter->setPen(QColor(150, 150, 150));
    painter->setFont(QFont("Microsoft YaHei", 9));
    QPointF vpOrigin = rect.topLeft() + QPointF(10, 20);
    painter->drawText(vpOrigin, "[-]");
    painter->drawText(vpOrigin + QPointF(35, 0), "[俯视]");
    painter->drawText(vpOrigin + QPointF(85, 0), "[二维线框]");

    // 右上角：ViewCube（简化版立方体）
    qreal cubeSize = 45;
    QPointF cubeCenter = rect.topRight() + QPointF(-cubeSize - 15, cubeSize + 15);
    painter->setPen(QPen(QColor(100, 100, 100), 1));
    painter->setBrush(QColor(60, 60, 60, 180));
    // 立方体顶面
    QPolygonF topFace;
    topFace << cubeCenter + QPointF(-cubeSize/2, -cubeSize/3)
            << cubeCenter + QPointF(0, -cubeSize/2)
            << cubeCenter + QPointF(cubeSize/2, -cubeSize/3)
            << cubeCenter + QPointF(0, -cubeSize/6);
    painter->drawPolygon(topFace);
    // 立方体前面
    QPolygonF frontFace;
    frontFace << cubeCenter + QPointF(-cubeSize/2, -cubeSize/3)
              << cubeCenter + QPointF(0, -cubeSize/6)
              << cubeCenter + QPointF(0, cubeSize/2)
              << cubeCenter + QPointF(-cubeSize/2, cubeSize/3);
    painter->setBrush(QColor(50, 50, 50, 180));
    painter->drawPolygon(frontFace);
    // 立方体右面
    QPolygonF rightFace;
    rightFace << cubeCenter + QPointF(0, -cubeSize/6)
              << cubeCenter + QPointF(cubeSize/2, -cubeSize/3)
              << cubeCenter + QPointF(cubeSize/2, cubeSize/3)
              << cubeCenter + QPointF(0, cubeSize/2);
    painter->setBrush(QColor(40, 40, 40, 180));
    painter->drawPolygon(rightFace);
    // "上"字
    painter->setPen(QColor(200, 200, 200));
    painter->setFont(QFont("Microsoft YaHei", 8, QFont::Bold));
    painter->drawText(cubeCenter + QPointF(-6, -cubeSize/4), "上");

    // 绘制捕捉标记
    if (m_snapManager && m_snapManager->hasSnap()) {
        m_snapManager->drawSnapMarker(painter);
    }
    // 绘制工具覆盖层
    if (m_currentTool) {
        m_currentTool->drawOverlay(painter);
    }
}

void CadView::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        QPointF worldPos = mapToScene(event->pos());
        QList<QGraphicsItem*> items = scene()->items(worldPos);
        for (auto item : items) {
            if (auto blockRef = dynamic_cast<Zhifen::BlockReference*>(item)) {
                // 双击块引用，发出信号通知主窗口编辑属性
                emit blockDoubleClicked(blockRef);
                return;
            }
        }
    }
    QGraphicsView::mouseDoubleClickEvent(event);
}
