#ifndef LINETOOL_H
#define LINETOOL_H

#include "tool.h"
#include <QPointF>
#include <QList>

class LineTool : public Tool
{
    Q_OBJECT
public:
    explicit LineTool(CadView *view, QObject *parent = nullptr);

    QString name() const override { return "直线"; }

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void drawOverlay(QPainter *painter) override;
    void deactivate() override;

private:
    QList<QPointF> m_points;
    QPointF m_currentPos;
    bool m_hasStart = false;
};

#endif // LINETOOL_H
