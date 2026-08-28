#ifndef QUERYTOOL_H
#define QUERYTOOL_H
#include "tool.h"
#include <QPointF>
class QueryTool : public Tool
{
    Q_OBJECT
public:
    enum QueryType { Distance, Area, Point };
    explicit QueryTool(CadView *view, QueryType type = Distance, QObject *parent = nullptr);
    QString name() const override;
    void setQueryType(QueryType type) { m_type = type; m_step = 0; }
    QueryType queryType() const { return m_type; }
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void drawOverlay(QPainter *painter) override;
    void deactivate() override;
signals:
    void queryResult(const QString &result);
private:
    QueryType m_type;
    QPointF m_p1, m_p2, m_currentPos;
    int m_step = 0;
};
#endif
