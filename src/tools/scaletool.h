#ifndef SCALETOOL_H
#define SCALETOOL_H
#include "tool.h"
#include <QPointF>
class QGraphicsItem;
class ScaleTool : public Tool
{
    Q_OBJECT
public:
    explicit ScaleTool(CadView *view, QObject *parent = nullptr);
    QString name() const override { return "缩放"; }
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
    QPointF scalePoint(const QPointF &p, const QPointF &center, qreal factor);
};
#endif
