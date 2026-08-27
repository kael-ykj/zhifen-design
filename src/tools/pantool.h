#ifndef PANTOOL_H
#define PANTOOL_H

#include "tool.h"

class PanTool : public Tool
{
    Q_OBJECT
public:
    explicit PanTool(CadView *view, QObject *parent = nullptr);

    QString name() const override { return "平移"; }
    QCursor cursor() const override { return Qt::OpenHandCursor; }

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    bool m_panning = false;
    QPoint m_lastPos;
};

#endif // PANTOOL_H
