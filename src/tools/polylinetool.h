#ifndef POLYLINETOOL_H
#define POLYLINETOOL_H
#include "tool.h"
#include <QPointF>
#include <QPolygonF>
class PolylineTool : public Tool
{
    Q_OBJECT
public:
    explicit PolylineTool(CadView *view, QObject *parent = nullptr);
    QString name() const override { return "多段线"; }
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void drawOverlay(QPainter *painter) override;
    void deactivate() override;
private:
    QPolygonF m_points;
    QPointF m_currentPos;
    bool m_hasStart = false;
};
#endif
