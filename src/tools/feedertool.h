#ifndef FEEDERTOOL_H
#define FEEDERTOOL_H

#include "tool.h"
#include "../entities/feederitem.h"
#include <QPointF>
#include <QPolygonF>

class FeederTool : public Tool
{
    Q_OBJECT
public:
    explicit FeederTool(CadView *view, QObject *parent = nullptr);
    QString name() const override { return "馈线绘制"; }
    void setFeederType(FeederItem::FeederType type) { m_feederType = type; }
    FeederItem::FeederType feederType() const { return m_feederType; }

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void drawOverlay(QPainter *painter) override;
    void deactivate() override;

private:
    FeederItem::FeederType m_feederType = FeederItem::Feeder_1_2;
    QPolygonF m_points;
    QPointF m_currentPos;
    bool m_drawing = false;
};

#endif // FEEDERTOOL_H
