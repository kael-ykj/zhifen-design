#ifndef ZOOMTOOL_H
#define ZOOMTOOL_H

#include "tool.h"
#include <QPoint>

class ZoomTool : public Tool
{
    Q_OBJECT
public:
    explicit ZoomTool(CadView *view, QObject *parent = nullptr);

    QString name() const override { return "缩放"; }

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void drawOverlay(QPainter *painter) override;

private:
    bool m_selecting = false;
    QPoint m_startPos;
    QPoint m_currentPos;
};

#endif // ZOOMTOOL_H
