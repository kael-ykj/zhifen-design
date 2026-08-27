#ifndef DIMENSIONTOOL_H
#define DIMENSIONTOOL_H
#include "tool.h"
#include <QPointF>
class DimensionTool : public Tool
{
    Q_OBJECT
public:
    explicit DimensionTool(CadView *view, QObject *parent = nullptr);
    QString name() const override { return "标注"; }
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void drawOverlay(QPainter *painter) override;
    void deactivate() override;
private:
    QPointF m_p1, m_p2, m_dimPos, m_currentPos;
    int m_step = 0;
};
#endif
