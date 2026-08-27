#ifndef RECTANGLETOOL_H
#define RECTANGLETOOL_H
#include "tool.h"
#include <QPointF>
class RectangleTool : public Tool
{
    Q_OBJECT
public:
    explicit RectangleTool(CadView *view, QObject *parent = nullptr);
    QString name() const override { return "矩形"; }
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void drawOverlay(QPainter *painter) override;
    void deactivate() override;
private:
    QPointF m_p1, m_currentPos;
    bool m_hasStart = false;
};
#endif
