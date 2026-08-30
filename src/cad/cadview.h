#ifndef CADVIEW_H
#define CADVIEW_H

#include <QGraphicsView>
#include <QMenu>
#include <QPointF>
#include <QPoint>

class CadScene;
class Tool;
class SnapManager;
class DynamicInput;
namespace Zhifen { class BlockReference; }
namespace Zhifen { class GripManager; struct GripPoint; }
class Document;

class CadView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit CadView(QWidget *parent = nullptr);

    void setCadScene(CadScene *scene);
    void setDocument(Document *doc) { m_document = doc; }
    void setCurrentTool(Tool *tool) { m_currentTool = tool; }
    Tool* currentTool() const { return m_currentTool; }
    SnapManager* snapManager() const { return m_snapManager; }

    // 视图控制
    void zoomExtents();
    void zoomIn();
    void zoomOut();
    void zoomAt(const QPoint &screenPos, qreal factor);

    // 坐标转换
    QPointF screenToWorld(const QPoint &screenPos) const;
    QPoint worldToScreen(const QPointF &worldPos) const;

    // 正交模式
    void setOrthoMode(bool enabled) { m_orthoMode = enabled; }
    void setDynamicInputEnabled(bool enabled) { m_dynamicInputEnabled = enabled; }
    bool dynamicInputEnabled() const { return m_dynamicInputEnabled; }
    bool orthoMode() const { return m_orthoMode; }

signals:
    void coordinateChanged(const QPointF &worldPos);
    void toolFinished();
    void blockDoubleClicked(Zhifen::BlockReference *block);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void drawForeground(QPainter *painter, const QRectF &rect) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    Document *m_document = nullptr;
    Tool *m_currentTool = nullptr;
    SnapManager *m_snapManager = nullptr;
    Zhifen::GripManager *m_gripManager = nullptr;
    bool m_gripDragging = false;
    Zhifen::GripPoint *m_draggedGrip = nullptr;
    DynamicInput *m_dynamicInput = nullptr;
    bool m_dynamicInputEnabled = true;
    bool m_orthoMode = false;
    bool m_panning = false;
    QPoint m_panStart;
    QPointF m_lastWorldPos;
};

#endif // CADVIEW_H
