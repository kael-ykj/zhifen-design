#ifndef CIRCLETOOL_H
#define CIRCLETOOL_H

#include "tool.h"
#include <QPointF>

class CircleTool : public Tool
{
    Q_OBJECT
public:
    explicit CircleTool(CadView *view, QObject *parent = nullptr);

    QString name() const override { return "圆"; }

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void drawOverlay(QPainter *painter) override;
    void deactivate() override;

private:
    QPointF m_center;
    QPointF m_currentPos;
    bool m_hasCenter = false;
};

#endif // CIRCLETOOL_H
