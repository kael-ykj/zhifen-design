#ifndef TEXTTOOL_H
#define TEXTTOOL_H
#include "tool.h"
#include <QPointF>
class TextTool : public Tool
{
    Q_OBJECT
public:
    explicit TextTool(CadView *view, QObject *parent = nullptr);
    QString name() const override { return "文字"; }
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void drawOverlay(QPainter *painter) override;
    void deactivate() override;
private:
    QPointF m_pos;
    bool m_hasPos = false;
    QString m_text;
};
#endif
