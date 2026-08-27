#ifndef COPYTOOL_H
#define COPYTOOL_H
#include "tool.h"
#include <QPointF>
#include <QList>
class QGraphicsItem;
class CopyTool : public Tool
{
    Q_OBJECT
public:
    explicit CopyTool(CadView *view, QObject *parent = nullptr);
    QString name() const override { return "复制"; }
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void drawOverlay(QPainter *painter) override;
    void deactivate() override;
private:
    QPointF m_basePoint, m_currentPos;
    bool m_hasBase = false;
    QList<QGraphicsItem*> m_selected;
};
#endif
