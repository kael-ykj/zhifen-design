#ifndef ROTATETOOL_H
#define ROTATETOOL_H
#include "tool.h"
#include <QPointF>
class QGraphicsItem;
class RotateTool : public Tool
{
    Q_OBJECT
public:
    explicit RotateTool(CadView *view, QObject *parent = nullptr);
    QString name() const override { return "旋转"; }
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void drawOverlay(QPainter *painter) override;
    void deactivate() override;
private:
    QPointF m_basePoint, m_refPoint, m_currentPos;
    int m_step = 0;
    QList<QGraphicsItem*> m_selected;
    QPointF rotatePoint(const QPointF &p, const QPointF &center, qreal angleRad);
};
#endif
