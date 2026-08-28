#ifndef EXPLODETOOL_H
#define EXPLODETOOL_H
#include "tool.h"
#include <QPointF>
class QGraphicsItem;
class ExplodeTool : public Tool
{
    Q_OBJECT
public:
    explicit ExplodeTool(CadView *view, QObject *parent = nullptr);
    QString name() const override { return "分解"; }
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void drawOverlay(QPainter *painter) override;
    void deactivate() override;
private:
    QPointF m_currentPos;
    void explodeItem(QGraphicsItem *item);
};
#endif
