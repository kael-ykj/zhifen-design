#ifndef MIRRORTOOL_H
#define MIRRORTOOL_H
#include "tool.h"
#include <QPointF>
#include <QList>
class QGraphicsItem;
class MirrorTool : public Tool
{
    Q_OBJECT
public:
    explicit MirrorTool(CadView *view, QObject *parent = nullptr);
    QString name() const override { return "镜像"; }
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void drawOverlay(QPainter *painter) override;
    void deactivate() override;
private:
    QPointF m_p1, m_p2, m_currentPos;
    int m_step = 0; // 0:选第一点, 1:选第二点, 2:确认
    QList<QGraphicsItem*> m_selected;
    bool m_deleteSource = false;
};
#endif
