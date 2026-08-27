#ifndef SELECTTOOL_H
#define SELECTTOOL_H

#include "tool.h"

class SelectTool : public Tool
{
    Q_OBJECT
public:
    explicit SelectTool(CadView *view, QObject *parent = nullptr);

    QString name() const override { return "选择"; }
    QCursor cursor() const override { return Qt::CrossCursor; }

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void drawOverlay(QPainter *painter) override;

private:
    bool m_selecting = false;
    QPoint m_startScreen;
    QPoint m_currentScreen;
    bool m_moveMode = false;
    QPointF m_moveStart;
};

#endif // SELECTTOOL_H
