#ifndef DIMENSIONTOOL_H
#define DIMENSIONTOOL_H
#include "tool.h"
#include "../entities/dimensionitem.h"
#include <QPointF>
class DimensionTool : public Tool
{
    Q_OBJECT
public:
    explicit DimensionTool(CadView *view, DimensionItem::DimType type = DimensionItem::Linear, QObject *parent = nullptr);
    QString name() const override;
    void setDimType(DimensionItem::DimType type) { m_dimType = type; m_step = 0; }
    DimensionItem::DimType dimType() const { return m_dimType; }
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void drawOverlay(QPainter *painter) override;
    void deactivate() override;
private:
    DimensionItem::DimType m_dimType;
    QPointF m_p1, m_p2, m_dimPos, m_currentPos;
    int m_step = 0;
    DimensionItem *m_preview = nullptr;
};
#endif
