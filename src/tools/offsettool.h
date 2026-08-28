#ifndef OFFSETTOOL_H
#define OFFSETTOOL_H
#include "tool.h"
#include <QPointF>
class QGraphicsItem;
class OffsetTool : public Tool
{
    Q_OBJECT
public:
    explicit OffsetTool(CadView *view, QObject *parent = nullptr);
    QString name() const override { return "偏移"; }
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void drawOverlay(QPainter *painter) override;
    void deactivate() override;
private:
    qreal m_distance = 10.0;
    QPointF m_p1, m_currentPos;
    QGraphicsItem *m_selected = nullptr;
    int m_step = 0; // 0:指定距离, 1:选对象, 2:指定侧
    void offsetItem(QGraphicsItem *item, const QPointF &sidePoint);
};
#endif
